(module 5L-Kernel (lib "lispish.ss" "5L")

  ;; Import %call-5l-prim from the engine.
  (require #%fivel-engine)

  ;; Get begin/var, and re-export it.
  (require (lib "begin-var.ss" "5L"))
  (provide begin/var)
  (provide define/var)
  
  ;; Get hooks, and re-export them.
  (require (lib "hook.ss" "5L"))
  (provide (all-from (lib "hook.ss" "5L")))

  ;; Get format-result-values.
  (require (lib "trace.ss" "5L"))

  ;; Require our macro-related helpers.
  (require-for-syntax (lib "capture.ss" "5L"))


  ;;=======================================================================
  ;;  Built-in Types
  ;;=======================================================================
  ;;  These methods implement various "built-in" types that are known to
  ;;  the 5L engine.  They should *never* raise errors, because they're
  ;;  called directly from C++ code that isn't prepared to cope with Scheme
  ;;  errors.

  (provide <point> (rename make-point point) point?
           point-x set-point-x! point-y set-point-y!

           <rect> (rename make-rect rect) rect?
           rect-left set-rect-left! rect-top set-rect-top!
           rect-right set-rect-right! rect-bottom set-rect-bottom!

           <color> (rename make-color-opt-alpha color) color?
           color-red set-color-red! color-green set-color-green!
           color-blue set-color-blue! color-alpha set-color-alpha!

           <percent> (rename make-percent percent) percent? percent-value
           
           <polygon> polygon polygon? 
           polygon-vertices set-polygon-vertices!

           <shape> shape?)
  
  (defclass <point> ()
    x y)

  (defclass <shape> ())

  (defclass <rect> (<shape>)
    left top right bottom)

  (defclass <color> ()
    red green blue alpha)

  (define (make-color-opt-alpha r g b &opt (a 0))
    (make-color r g b a))

  (defclass <percent> () 
    value)

  (defclass <polygon> (<shape>)
    vertices)

  (define (polygon &rest args)
    (make-polygon args))


  ;;=======================================================================
  ;;  Assertions
  ;;=======================================================================

  (provide assert)

  (define (%kernel-assert label value)
    (when (not value)
      (fatal-error (cat "Assertion failure: " label))))
  
  (define-syntax assert
    (syntax-rules ()
      [(assert cond)
       (%kernel-assert 'cond cond)]))


  ;;=======================================================================
  ;;  Utility Functions
  ;;=======================================================================

  (provide foreach member? value->string cat label with-errors-blocked)

  ;;; Run a body once for each item in a list.
  ;;;
  ;;; @syntax (foreach [name list] body ...)
  ;;; @param NAME name The variable to use as the item name.
  ;;; @param LIST list The list from which to get the items.
  ;;; @param BODY body The code to run for each list item.
  (define-syntax foreach
    (syntax-rules ()
      [(foreach [name lst] body ...)
       (let loop [[remaining lst]]
         (unless (null? remaining)
           (let [[name (car remaining)]]
             (begin/var body ...))
           (loop (cdr remaining))))]))
  
  (define (member? item list)
    (if (null? list)
        #f
        (if (equal? item (car list))
            #t
            (member? item (cdr list)))))
  
  (define (value->string value)
    (if (string? value)
        value
        (let ((str-port (open-output-string)))
          (write value str-port)
          (get-output-string str-port))))
  
  (define (cat . values)
    (if (not (null? values))
        (string-append (value->string (car values)) (apply cat (cdr values)))
        ""))

  ;; Already provided by Swindle, I think.
  ;;(define (keyword? value)
  ;;  (and (symbol? value)
  ;;       (let [[str (symbol->string value)]]
  ;;         (and (> 0 (string-length str))
  ;;              (eq? (string-ref str 0) #\:)))))

  (define (keyword-name value)
    (assert (keyword? value))
    (let [[str (symbol->string value)]]
      (string->symbol (substring str 1 (string-length str)))))

  (define-syntax label
    (syntax-rules ()
      [(label name body ...)
       (call-with-escape-continuation (lambda (name)
                                        (begin/var body ...)))]))

  (define (call-with-errors-blocked report-func thunk)
    (let* ((result (with-handlers ([void (lambda (exn) (cons #f exn))])
                     (cons #t (thunk))))
           (good? (car result))
           (exn-or-value (cdr result)))
      (if good?
          exn-or-value
          (begin
            (report-func (exn-message exn-or-value))
            #f))))

  (define-syntax with-errors-blocked
    (syntax-rules ()
      [(with-errors-blocked (report-func) body ...)
       (call-with-errors-blocked report-func
                                 (lambda () (begin/var body ...)))]))

  (define-syntax with-values
    (syntax-rules ()
      [(with-values values expr body ...)
       (call-with-values (lambda () expr) (lambda values body ...))]))
             

  ;;=======================================================================
  ;;  Standard Hooks
  ;;=======================================================================
  
  (provide *enter-card-hook* *exit-card-hook*
           *card-body-finished-hook* *before-draw-hook*)

  ;; Called before running each card.
  (define *enter-card-hook* (make-hook 'enter-card))

  ;; Called immediately before moving to a new card.
  (define *exit-card-hook* (make-hook 'exit-card))

  ;; Called after running the body of a each card, if the body exits
  ;; normally (not by jumping, etc.).
  (define *card-body-finished-hook* (make-hook 'card-body-finished))

  ;; Called before *most* screen redraws.
  (define *before-draw-hook* (make-hook 'before-draw))


  ;;=======================================================================
  ;;  Kernel Entry Points
  ;;=======================================================================
  ;;  The '%kernel-' methods are called directly by the 5L engine.  They
  ;;  shouldn't raise errors, because they're called directly from C++ code
  ;;  that doesn't want to catch them (and will, in fact, quit the
  ;;  program).
  ;;
  ;;  The theory behind these functions is documented in detail in
  ;;  TInterpreter.h.

  (define (%kernel-run)
    ;; The workhorse function.  We get called to manage the main event
    ;; loop, and we provide support code for handling jumps, STOPPING
    ;; the interpreter, idling after the end of each card, and quiting
    ;; the interpreter.  This code needs to be understood in
    ;; conjuction with *%kernel-state*, the functions that manipulate
    ;; it, and the callback system.  Yes, it's ugly--but it lets us
    ;; get the semantics we want without writing an entire interpreter
    ;; from scratch.
    (with-errors-blocked (fatal-error)
      (label exit-interpreter
        (fluid-let ((*%kernel-exit-interpreter-func* exit-interpreter))
          (let ((jump-card #f))
            (let loop []
              (label exit-to-top
                (with-errors-blocked (non-fatal-error)
                  (fluid-let ((*%kernel-exit-to-top-func* exit-to-top))
                    (idle)
                    (cond
                     [jump-card
                      (run-card (find-card jump-card))]
                     [#t
                      ;; Highly optimized do-nothing loop. :-)  This
                      ;; is a GC optimization designed to prevent the
                      ;; interpreter from allocating memory like a crazed
                      ;; maniac while the user's doing nothing.  If we
                      ;; removed this block, we'd have to perform a lot
                      ;; of LABEL and FLUID-LET statements, which are
                      ;; extremely expensive in quantities of 1,000.
                      (let idle-loop []
                        (unless (eq? *%kernel-state* 'JUMPING)
                          (if (%kernel-stopped?)
                              (blocking-idle)
                              (idle))
                          (idle-loop)))]))))
              (set! jump-card #f)
              (when (eq? *%kernel-state* 'JUMPING)
                ;; Handle a jump by setting jump-card for our next goaround.
                (set! jump-card *%kernel-jump-card*))
              (%kernel-maybe-clear-state)
              (loop)))))
      (%kernel-maybe-clear-state)
      (%kernel-clear-timeout)))

  (define (%kernel-kill-interpreter)
    (%kernel-set-state 'INTERPRETER-KILLED))

  (define (%kernel-stop)
    (%kernel-set-state 'STOPPING))

  (define (%kernel-go)
    (when (%kernel-stopped?)
      (%kernel-set-state 'NORMAL)))

  (define (%kernel-stopped?)
    (or (eq? *%kernel-state* 'STOPPING)
        (eq? *%kernel-state* 'STOPPED)))
  
  (define (%kernel-pause)
    (%kernel-die-if-callback '%kernel-pause)
    (%kernel-set-state 'PAUSED))

  (define (%kernel-wake-up)
    (%kernel-die-if-callback '%kernel-wake-up)
    (when (%kernel-paused?)
      (%kernel-clear-state)))

  (define (%kernel-paused?)
    (eq? *%kernel-state* 'PAUSED))

  (define (%kernel-timeout card-name seconds)
    (%kernel-die-if-callback '%kernel-timeout)
    (%kernel-set-timeout (+ (current-milliseconds) (* seconds 1000))
                         (lambda () (jump (find-card card-name)))))

  (define (%kernel-nap tenths-of-seconds)
    (%kernel-die-if-callback '%kernel-nap)
    (set! *%kernel-nap-time* (+ (current-milliseconds)
                                (* tenths-of-seconds 100)))
    (%kernel-set-state 'NAPPING))

  (define (%kernel-napping?)
    (eq? *%kernel-state* 'NAPPING))

  (define (%kernel-kill-nap)
    (%kernel-die-if-callback '%kernel-kill-nap)
    (when (%kernel-napping?)
      (%kernel-clear-state)))

  (define (%kernel-kill-current-card)
    (%kernel-set-state 'CARD-KILLED))

  (define (%kernel-jump-to-card-by-name card-name)
    (set! *%kernel-jump-card* card-name)
    (%kernel-set-state 'JUMPING))

  (define (%kernel-current-card-name)
    (if *%kernel-current-card*
        (value->string (%kernel-card-name *%kernel-current-card*))
        ""))

  (define (%kernel-previous-card-name)
    (if *%kernel-previous-card*
        (value->string (%kernel-card-name *%kernel-previous-card*))
        ""))

  (define (%kernel-valid-card? card-name)
    (card-exists? card-name))

  (define (%kernel-element-deleted element-name)
    (with-errors-blocked (non-fatal-error)
      (delete-element-info element-name)))

  (define (%kernel-eval expression)
    (let [[ok? #t] [result "#<did not return from jump>"]]

      ;; Jam everything inside a callback so JUMP, etc., work
      ;; as the user expects.  Do some fancy footwork to store the
      ;; return value(s) and return them correctly.  This code is ugly
      ;; because I'm too lazy to redesign the callback architecture
      ;; to make it pretty.
      (%kernel-run-as-callback
       ;; Our callback.
       (lambda ()
         (call-with-values
          (lambda () (eval (read (open-input-string expression))))
          (lambda r (set! result (apply format-result-values r)))))

       ;; Our error handler.
       (lambda (error-msg)
         (set! ok? #f)
         (set! result error-msg)))
      
      ;; Return the result.
      (cons ok? result)))

  (define (%kernel-run-callback function args)
    (%kernel-run-as-callback (lambda () (apply function args))
                             non-fatal-error))

  (define (%kernel-reverse! l)
    (reverse! l))


  ;;=======================================================================
  ;;  Internal State
  ;;=======================================================================
  ;;  When the interpreter returns from a primitive call (including a
  ;;  primitive call to run the idle loop), it can be in one of a number of
  ;;  states:
  ;;
  ;;    NORMAL:   The interpreter is running code normally, or has no code
  ;;              to run.  The default.
  ;;    STOPPING: The kernel is about to stop.  We use a two-phase stop
  ;;              system--STOPPING indicates we should escape to the
  ;;              top-level loop, and STOPPED indicates that we should
  ;;              efficiently busy-wait at the top-level.
  ;;    STOPPED:  The kernel has been stopped.
  ;;    PAUSED:   The interpreter should pause the current card until it is
  ;;              told to wake back up.  This is used for modal text
  ;;              entry fields, (wait ...), and similar things.
  ;;    JUMPING:  The interpreter should execute a jump.
  ;;    NAPPING:  The interpreter should pause the current card for the
  ;;              specified number of milliseconds.
  ;;    CARD-KILLED: The interpreter should stop executing the current
  ;;               card, and return to the top-level loop.
  ;;    INTERPRETER-KILLED: The interpreter should exit.
  ;;
  ;;  Callbacks are slightly special, however--see the code for details.
  
  (provide call-at-safe-time)

  ;; The most important global state variables.
  (define *%kernel-running-callback?* #f)
  (define *%kernel-state* 'NORMAL)

  ;; Some slightly less important global state variables.  See the
  ;; functions which define and use them for details.
  (define *%kernel-jump-card* #f)
  (define *%kernel-timeout* #f)
  (define *%kernel-timeout-thunk* #f)
  (define *%kernel-nap-time* #f)

  ;; Deferred thunks are used to implement (deferred-callback () ...).
  ;; See call-at-safe-time for details.
  (define *%kernel-running-deferred-thunks?* #f)
  (define *%kernel-deferred-thunk-queue* '())
  
  ;; These are bound at the top level in %kernel-run, and get
  ;; temporarily rebound during callbacks.  We use
  ;; *%kernel-exit-interpreter-func* to help shut down the
  ;; interpreter, and *%kernel-exit-to-top-func* to help implement
  ;; anything that needs to bail out to the top-level loop (usually
  ;; after setting up some complex state).  See the functions which define
  ;; and call these functions for more detail.
  (define *%kernel-exit-interpreter-func* #f)
  (define *%kernel-exit-to-top-func* #f)
  
  (define (%kernel-die-if-callback name)
    (if *%kernel-running-callback?*
        (throw (cat "Cannot call " name " from within callback."))))

  (define (%kernel-clear-timeout)
    (set! *%kernel-timeout* #f)
    (set! *%kernel-timeout-thunk* #f))

  (define (%kernel-set-timeout time thunk)
    (when *%kernel-timeout*
      (debug-caution "Installing new timeout over previously active one."))
    (set! *%kernel-timeout* time)
    (set! *%kernel-timeout-thunk* thunk))
  
  (define (%kernel-check-timeout)
    ;; If we have a timeout to run, and it's a safe time to run it, do so.
    (if (%kernel-stopped?)
        (%kernel-clear-timeout)
        (unless *%kernel-running-callback?*
          (when (and *%kernel-timeout*
                     (>= (current-milliseconds) *%kernel-timeout*))
            (let ((thunk *%kernel-timeout-thunk*))
              (%kernel-clear-timeout)
              (thunk))))))

  (define (%kernel-safe-to-run-deferred-thunks?)
    ;; Would now be a good time to run deferred thunks?  Wait until
    ;; nothing exciting is happening.  See call-at-safe-time.
    (and (not *%kernel-running-callback?*)
         (not *%kernel-running-deferred-thunks?*)
         (member? *%kernel-state* '(NORMAL PAUSED NAPPING))))

  (define (call-at-safe-time thunk)
    ;; Make sure we run 'thunk' at the earliest safe time, but not
    ;; in a callback.  This can be used to defer calls to video, input,
    ;; and other functions which can't be called from a callback.
    (if (%kernel-safe-to-run-deferred-thunks?)
        (thunk)
        (set! *%kernel-deferred-thunk-queue*
              (cons thunk *%kernel-deferred-thunk-queue*)))
    #f)
    
  (define (%kernel-check-deferred)
    ;; If the interpreter has stopped, cancel any deferred thunks.  (See
    ;; call-at-safe-time.)  This function won't call any deferred thunks
    ;; unless it is safe to do so.
    (when (%kernel-stopped?)
      (set! *%kernel-deferred-thunk-queue* '()))   

    ;; Run any deferred thunks.
    (unless (or (null? *%kernel-deferred-thunk-queue*)
                (not (%kernel-safe-to-run-deferred-thunks?)))

      ;; Make a copy of the old queue and clear the global variable
      ;; (which may be updated behind our backs).
      (let [[items (reverse *%kernel-deferred-thunk-queue*)]]
        (set! *%kernel-deferred-thunk-queue* '())

        ;; Run every thunk in the queue, in order.
        (fluid-let [[*%kernel-running-deferred-thunks?* #t]]
          (foreach [item items]
            (item))))

      ;; Check to see if any new items appeared in the queue while we
      ;; were running the first batch.
      (%kernel-check-deferred)))

  (define (%kernel-clear-state)
    ;; This is the version that we want to call from most places to get the
    ;; current state set back to normal.
    (assert (not (or (eq? *%kernel-state* 'STOPPING) 
                     (eq? *%kernel-state* 'STOPPED))))
    (set! *%kernel-state* 'NORMAL)
    (set! *%kernel-jump-card* #f))

  (define (%kernel-maybe-clear-state)
    ;; This should only ever be called from the main loop, because then
    ;; stopping has finished and the interpreter is in a stopped state. 
    ;; Everyone else should only ever call %kernel-clear-state
    (case *%kernel-state*
      [[STOPPING]
       (set! *%kernel-state* 'STOPPED)]
      [[STOPPED]
       #f]
      [else
       (set! *%kernel-state* 'NORMAL)])
    (set! *%kernel-jump-card* #f))
  
  (define (%kernel-run-as-callback thunk error-handler)
    ;; This function is in charge of running callbacks from C++ into
    ;; Scheme.  These include simple callbacks and anything evaled from
    ;; C++.  When we're in a callback, we need to install special values
    ;; of *%kernel-exit-to-top-func* and *%kernel-exit-interpreter-func*,
    ;; because the values installed by %kernel-run can't be invoked without
    ;; "throwing" across C++ code, which isn't allowed (and which would
    ;; be disasterous).  Furthermore, we must trap all errors, because
    ;; our C++-based callers don't want to deal with Scheme exceptions.
    (if (eq? *%kernel-state* 'INTERPRETER-KILLED)
        (5l-log "Skipping callback because interpreter is being shut down")
        (let [[saved-kernel-state *%kernel-state*]]
          (set! *%kernel-state* 'NORMAL)
          (label exit-callback
            ;; TODO - Can we have better error handling?
            (with-errors-blocked (error-handler)
              (fluid-let [[*%kernel-exit-to-top-func* exit-callback]
                          [*%kernel-exit-interpreter-func* exit-callback]
                          [*%kernel-running-callback?* #t]]
                (thunk))))
          (if (eq? *%kernel-state* 'NORMAL)
              (set! *%kernel-state* saved-kernel-state)))))

  (define (%kernel-set-state state)
    (set! *%kernel-state* state))

  (define (%kernel-check-state)
    ;; This function is called immediately after returning from any
    ;; primitive call (including idle).  It's job is to check flags
    ;; set by the engine, handle any deferred callbacks, check
    ;; timeouts, and generally bring the Scheme world back into sync
    ;; with everybody else.
    ;; 
    ;; Since this is called after idle, it needs to be *extremely*
    ;; careful about allocating memory.  See %kernel-run for more
    ;; discussion about consing in the idle loop (hint: it's bad).
    (%kernel-check-deferred) ; Should be the first thing we do.
    (unless *%kernel-running-callback?*
      (%kernel-check-timeout))
    (case *%kernel-state*
      [[NORMAL STOPPED]
       #f]
      [[STOPPING]
       (when *%kernel-exit-to-top-func*
             (*%kernel-exit-to-top-func* #f))]
      [[PAUSED]
       (%call-5l-prim 'schemeidle #f)
       (%kernel-check-state)]       ; Tail-call self without consing.
      [[NAPPING]
       (if (< (current-milliseconds) *%kernel-nap-time*)
           (begin
             (%call-5l-prim 'schemeidle #f)
             (%kernel-check-state)) ; Tail call self without consing.
           (%kernel-clear-state))]
      [[JUMPING]
       (when *%kernel-exit-to-top-func*
             (*%kernel-exit-to-top-func* #f))]
      [[CARD-KILLED]
       (when *%kernel-exit-to-top-func*
             (*%kernel-exit-to-top-func* #f))]
      [[INTERPRETER-KILLED]
       (*%kernel-exit-interpreter-func* #f)]
      [else
       (fatal-error "Unknown interpreter state")]))


  ;;=======================================================================
  ;;  Core 5L API
  ;;=======================================================================
  ;;  We only declare a small number of primitives here, typically those
  ;;  which are needed by the kernel or which intimately depend on the
  ;;  kernel's inner workings.  The rest of these functions can be found
  ;;  in the 5L-API module.
  
  (provide call-5l-prim have-5l-prim? idle blocking-idle 5l-log
           debug-log caution debug-caution non-fatal-error fatal-error 
           engine-var set-engine-var! engine-var-exists?  throw 
           exit-script jump refresh)

  ;; C++ can't handle very large or small integers.  Here are the
  ;; typical limits on any modern platform.
  (define *32-bit-signed-min* -2147483648)
  (define *32-bit-signed-max* 2147483647)
  (define *32-bit-unsigned-min* 0)
  (define *32-bit-unsigned-max* 4294967295)

  (define (call-5l-prim . args)
    ;; Our high-level wrapper for %call-5l-prim (which is defined by
    ;; the engine).  You should almost always call this instead of
    ;; %call-5l-prim, because this function calls %kernel-check-state
    ;; to figure out what happened while we were in C++.
    (let ((result (apply %call-5l-prim args)))
      (%kernel-check-state)
      result))
  
  (define (have-5l-prim? name)
    (call-5l-prim 'haveprimitive name))
  
  (define (blocking-idle)
    (%kernel-die-if-callback 'idle)
    (call-5l-prim 'schemeidle #t))

  (define (idle)
    (%kernel-die-if-callback 'idle)
    (call-5l-prim 'schemeidle #f))
  
  (define (5l-log msg)
    (%call-5l-prim 'log '5L msg 'log))
  
  (define (debug-log msg)
    (%call-5l-prim 'log 'Debug msg 'log))
  
  (define (caution msg)
    (%call-5l-prim 'log '5L msg 'caution))
  
  (define (debug-caution msg)
    (%call-5l-prim 'log 'Debug msg 'caution))
  
  (define (non-fatal-error msg)
    (%call-5l-prim 'log '5L msg 'error))
  
  (define (fatal-error msg)
    (%call-5l-prim 'log '5L msg 'fatalerror))
  
  (define (engine-var name)
    (call-5l-prim 'get (if (string? name) (string->symbol name) name)))
  
  (define (set-engine-var! name value)
    ;; Set an engine variable.  This is a pain, because we have to play
    ;; along with the engine's lame type system.
    (let [[namesym (if (string? name) (string->symbol name) name)]
          [type
           (cond
            [(void? value) 'NULL]
            [(string? value) 'STRING]
            [(symbol? value) 'SYMBOL]
            [(and (integer? value) (exact? value))
             (cond
              [(<= *32-bit-signed-min* value *32-bit-signed-max*) 'LONG]
              [(<= *32-bit-unsigned-min* value *32-bit-unsigned-max*) 'ULONG]
              [else (throw (cat "Cannot store " value " in " name
                                " because it does fall between "
                                *32-bit-signed-min* " and "
                                *32-bit-unsigned-max* "."))])]
            [(number? value) 'DOUBLE]
            [(or (eq? value #t) (eq? value #f)) 'BOOLEAN]
            [else (throw (cat "Cannot store " value " in " name "."))])]]
      (if (eq? type 'NULL)
          (call-5l-prim 'settyped namesym type)
          (call-5l-prim 'settyped namesym type value))))

  (define (engine-var-exists? name)
    (call-5l-prim 'VariableInitialized name))
  
  (define (throw msg)
    ;; TODO - More elaborate error support.
    (non-fatal-error msg)
    (error msg))
  
  (define (exit-script)
    (call-5l-prim 'schemeexit))
  
  (define (jump-to-card card)
    (if (have-5l-prim? 'jump)
        (call-5l-prim 'jump (card-name card))
        (begin
          ;; If we don't have a JUMP primitive, fake it by hand.
          (set! *%kernel-jump-card* card)
          (%kernel-set-state 'JUMPING)
          (%kernel-check-state))))

  (define (refresh &key (transition 'none) (ms 500))
    ;; Refresh the screen by blitting dirty areas of our offscreen buffer
    ;; to the display.
    (call-hook-functions *before-draw-hook*)
    (if (have-5l-prim? 'refresh)
        (call-5l-prim 'refresh transition ms)))


  ;;=======================================================================
  ;;  Object Model
  ;;=======================================================================

  ;;-----------------------------------------------------------------------
  ;;  Events
  ;;-----------------------------------------------------------------------

  (provide on send-by-name send
           <event> event?
           <idle-event> idle-event?
           <char-event> char-event? event-character event-modifiers
           <mouse-event> mouse-event? event-position event-double-click?
           make-node-event-dispatcher ; semi-private
           )

  (define-syntax (expand-on stx)
    (syntax-case stx ()
      [(expand-on node name args . body)
       ;; Create a capture variable NEXT-HANDLER which will be visible in
       ;; BODY.  It's exceptionally evil to capture using BODY's context
       ;; instead of EXPAND-ON's context, but that's what we want.
       (with-syntax [[call-next-handler
                      (make-capture-var/ellipsis #'body 'call-next-handler)]]
         (quasisyntax/loc
          stx
          (register-event-handler node 'name
                                  (lambda (call-next-handler . args)
                                    . body))))]))

  (define-syntax on
    ;; This gets lexically overridden by expand-init-fn to refer
    ;; to nodes other than $root-node.
    (syntax-rules ()
      [(on . rest)
       (expand-on $root-node . rest)]))

  (define (register-event-handler node name handler)
    (debug-log (cat "Registering handler: " name " in " (node-full-name node)))

    ;; Keep track of whether we're handling expensive events.  We call
    ;; ENABLE-EXPENSIVE-EVENTS here, which is sufficient for <card> and
    ;; <element> nodes.  But since <card-group>s and <card-sequence>s stay
    ;; alive longer than a single card, we need to set
    ;; NODE-HAS-EXPENSIVE-HANDLERS?, which is used by
    ;; MAYBE-ENABLE-EXPENSIVE-EVENTS-FOR-CARD (on behalf of RUN-CARD) to do
    ;; the rest of our bookkeeping.
    (when (expensive-event? name)
      (set! (node-has-expensive-handlers? node) #t)
      (enable-expensive-events #t))

    ;; Update our handler table.
    (let* [[table (node-handlers node)]
           [old-handler (hash-table-get table name (lambda () #f))]]
      (if old-handler
          (hash-table-put! table name
                           ;; This is tricky--we need to replace the old
                           ;; handler with our new one.  To do this, we
                           ;; create a glue function to Do The Right Thing
                           ;; with the NEXT-HANDLER argument.
                           ;;
                           ;; TODO - We don't catch duplicate handlers
                           ;; within a single node or template (or at the
                           ;; top level).  This would be a Good Thing<tm>
                           ;; to do correctly.
                           (lambda (call-next-handler . args)
                             (apply handler
                                    (lambda ()
                                      (apply old-handler
                                             call-next-handler args))
                                    args)))
          (hash-table-put! table name handler))))

  (define (send/nonrecursive* call-next-handler node name . args)
    (let [[handler (hash-table-get (node-handlers node) name (lambda () #f))]]
      (if handler
          (apply handler call-next-handler args)
          (call-next-handler))))

  (define (send* call-next-handler node name . args)
    (let recurse [[node node]]
      (if (not node)
          (call-next-handler)
          (let [[new-call-next-handler
                 (lambda () (recurse (node-parent node)))]]
            (apply send/nonrecursive* new-call-next-handler node name args)))))

  (define (send-by-name node name . args)
    (define (no-handler)
      (error (cat "No handler for " name " on " (node-full-name node))))
    (apply send* no-handler node name args))

  (define-syntax send
    (syntax-rules ()
      [(send node name . args)
       (send-by-name node 'name . args)]))

  (defclass <event> ())
  (defclass <idle-event> (<event>))

  (defclass <char-event> (<event>)
    (character :accessor event-character)
    (modifiers :accessor event-modifiers))

  (defclass <mouse-event> (<event>)
    (position :accessor event-position)
    (double-click? :accessor event-double-click? :initvalue #f))

  (define (dispatch-event-to-node node name args)
    (let [[unhandled? #f]
          [event (case name
                   [[char] (make <char-event>
                             :character (string-ref (car args) 0)
                             :modifiers (cadr args))]
                   [[mouse-down]
                    (make <mouse-event>
                      :position (make-point (car args) (cadr args))
                      :double-click? (caddr args))]
                   [[mouse-up mouse-enter mouse-leave]
                    (make <mouse-event>
                      :position (make-point (car args) (cadr args)))]
                   [else
                    (non-fatal-error (cat "Unsupported event type: " name))])]]
      (define (no-handler)
        (set! unhandled? #t))
      (send* no-handler node name event)
      (set! (engine-var '_pass) unhandled?)))

  (define (dispatch-idle-event-to-active-nodes)
    (define event (make <idle-event>))
    (define (no-handler)
      (void))
    (define (send-idle node)
      (send/nonrecursive* no-handler node 'idle event))
    (let loop [[node (current-card)]]
      (when node
        (send-idle node)
        (loop (node-parent node))))
    (foreach [child (group-children (current-card))]
      (send-idle child)))  

  (define (dispatch-event-to-current-card name . args)
    (when (current-card)
      (if (eq? name 'idle)
          (dispatch-idle-event-to-active-nodes)
          (dispatch-event-to-node (current-card) name args))))

  (define (make-node-event-dispatcher node)
    (lambda (name . args)
      (dispatch-event-to-node node name args)))

  (define (expensive-event? name)
    ;; Some events are sent almost constantly, and cause us to allocate
    ;; memory too quickly.  This causes a performance loss.  To avoid
    ;; this performance loss, we only enable the sending of these events
    ;; if we believe there is a handler to receive them.
    (case name
      [[idle mouse-moved] #t]
      [else #f]))

  (define (enable-expensive-events enable?)
    (when (have-5l-prim? 'EnableExpensiveEvents)
      (call-5l-prim 'EnableExpensiveEvents enable?)))
  
  ;; Set up our event handling machinery.
  (enable-expensive-events #f)
  (when (have-5l-prim? 'RegisterEventDispatcher)
    (call-5l-prim 'RegisterEventDispatcher dispatch-event-to-current-card))
  

  ;;-----------------------------------------------------------------------
  ;;  Templates
  ;;-----------------------------------------------------------------------
  
  (provide prop-by-name prop)

  ;; These objects are distinct from every other object, so we use them as
  ;; unique values.
  (define $no-default (list 'no 'default))
  (define $no-such-key (list 'no 'such 'key))

  (defclass <template-prop-decl> ()
    (name      :type <symbol>)
    (type      :type <class>       :initvalue <object>)
    (label     :type <string>      :initvalue "")
    (default                       :initvalue $no-default))

  (defclass <template> ()
    (group      :type <symbol>     :initvalue 'none)
    (extends                       :initvalue #f)
    (prop-decls :type <list>       :initvalue '())
    (bindings-eval-fn :type <function>)
    (init-fn :type <function>   :initvalue (lambda (node) #f)))

  (defmethod (initialize (template <template>) initargs)
    (call-next-method)
    ;; Make sure templates only extend other templates in their own group.
    (assert (or (not (template-extends template))
                (eq? (template-group template)
                     (template-group (template-extends template))))))

  (define (node-bind-value! node name value)
    (if (node-has-value? node name)
        (error (cat "Duplicate property " name " on node "
                    (node-full-name node)))
        (hash-table-put! (node-values node) name value)))

  (define (node-has-value? node name)
    (not (eq? (hash-table-get (node-values node) name
                              (lambda () $no-such-key))
              $no-such-key)))

  (define (node-maybe-default-property! node prop-decl)
    (unless (node-has-value? node (template-prop-decl-name prop-decl))
      (node-bind-value! node
                        (template-prop-decl-name prop-decl)
                        (template-prop-decl-default prop-decl))))

  (define (node-bind-property-values! node template)
    (let [[bindings ((template-bindings-eval-fn template) node)]]
      (hash-table-for-each bindings
                           (lambda (k v) (node-bind-value! node k v))))
    (foreach [decl (template-prop-decls template)]
      (node-maybe-default-property! node decl)))

  (define (prop-by-name node name)
    ;; This function controls how we search for property bindings.  If
    ;; you want to change search behavior, here's the place to do it.
    (let [[value (hash-table-get (node-values node)
                                 name (lambda () $no-such-key))]]
      (if (not (eq? value $no-such-key))
          value
          (error "Unable to find template property" name))))

  (define-syntax prop
    (syntax-rules ()
      [(prop node name)
       (prop-by-name node 'name)]))

  (define (bindings->hash-table bindings)
    ;; Turns a keyword argument list into a hash table.
    (let [[result (make-hash-table)]]
      (let recursive [[b bindings]]
        (cond
         [(null? b)
          #f]
         [(null? (cdr b))
          (error "Bindings list contains an odd number of values:" bindings)]
         [(not (keyword? (car b)))
          (error "Expected keyword in bindings list, found:" (car b) bindings)]
         [else
          (hash-table-put! result (keyword-name (car b)) (cadr b))
          (recursive (cddr b))]))
      result))

  (define-syntax expand-prop-decls
    (syntax-rules ()
      [(expand-prop-decls)
       '()]
      [(expand-prop-decls (name keywords ...) rest ...)
       (cons (make <template-prop-decl> :name 'name keywords ...)
             (expand-prop-decls rest ...))]
      [(expand-prop-decls name rest ...)
       (cons (make <template-prop-decl> :name 'name)
             (expand-prop-decls rest ...))]))

  (define-syntax (expand-fn-with-self-and-prop-names stx)
    (syntax-case stx ()
      [(expand-fn-with-self self prop-decls . body)
       (begin

         ;; Bind each template property NAME to a call to (prop self
         ;; 'name), so it's convenient to access from with the init-fn.
         ;; We don't need to use capture variables for this, because
         ;; the programmer supplied the names--which means they're already
         ;; in the right context.
         (define (make-prop-bindings self-stx prop-decls-stx)
           (with-syntax [[self self-stx]]
             (datum->syntax-object
              stx
              (map (lambda (prop-stx)
                     (syntax-case prop-stx ()
                       [(name . rest)
                        (syntax/loc prop-stx [name (prop self name)])]
                       [name
                        (syntax/loc prop-stx [name (prop self name)])]))
                   (syntax->list prop-decls-stx)))))

         ;; We introduce a number of "capture" variables in BODY.  These
         ;; will be bound automagically within BODY without further
         ;; declaration.  See the PLT203 mzscheme manual for details.
         (quasisyntax/loc
          stx
          (lambda (self)
            (letsubst #,(make-prop-bindings #'self #'prop-decls)
              (begin/var . body)))))]))

  (define-syntax (expand-init-fn stx)
    (syntax-case stx ()
      [(expand-init-fn prop-decls . body)
         
       ;; We introduce a number of "capture" variables in BODY.  These
       ;; will be bound automagically within BODY without further
       ;; declaration.  See the PLT203 mzscheme manual for details.
       (with-syntax [[self (make-capture-var/ellipsis #'body 'self)]
                     [on   (make-capture-var/ellipsis #'body 'on)]]
         (quasisyntax/loc
          stx
          (expand-fn-with-self-and-prop-names self prop-decls
            (let-syntax [[on (syntax-rules ()
                               [(_ . rest)
                                (expand-on self . rest)])]]
              (begin/var . body)))))]))
  
  (define-syntax (expand-bindings-eval-fn stx)
    (syntax-case stx ()
      [(expand-bindings-eval-fn prop-decls . bindings)
       (with-syntax [[self (make-capture-var/ellipsis #'body 'self)]]
         (quasisyntax/loc
          stx
          (expand-fn-with-self-and-prop-names self prop-decls
            (bindings->hash-table (list . bindings)))))]))

  (define-syntax define-template
    (syntax-rules (:template)
      [(define-template group name prop-decls
                              (:template extended . bindings)
         . body)
       (define name (make <template>
                      :group      group
                      :extends    extended
                      :bindings-eval-fn (expand-bindings-eval-fn prop-decls 
                                                                 . bindings)
                      :prop-decls (expand-prop-decls . prop-decls)
                      :init-fn    (expand-init-fn prop-decls . body)))]
      [(define-template group name prop-decls bindings . body)
       (define-template group name prop-decls (:template #f . bindings)
         . body)]))

  (define-syntax define-template-definer
    (syntax-rules ()
      [(define-template-definer definer-name group)
       (define-syntax definer-name
         (syntax-rules ()
           [(definer-name . rest)
            (define-template 'group . rest)]))]))

  ;;-----------------------------------------------------------------------
  ;;  Nodes
  ;;-----------------------------------------------------------------------
  ;;  A program is (among other things) a tree of nodes.  The root node
  ;;  contains <card>s and <card-groups>.  <card>s contain <element>s.

  (provide <node> node? node-name node-full-name node-parent find-node @
           elem-or-name-hack)

  (defclass <node> (<template>)
    name
    parent
    (has-expensive-handlers? :type <boolean> :initvalue #f)
    (handlers :type <hash-table> :initializer (lambda () (make-hash-table)))
    (values   :type <hash-table> :initializer (lambda () (make-hash-table)))
    )

  (defmethod (initialize (node <node>) initargs)
    (call-next-method)
    (when (node-parent node)
      (group-add-child! (node-parent node) node)))

  (define (node-full-name node)
    ;; Join together local names with "/" characters.
    (let [[parent (node-parent node)]]
      (if (and parent (not (eq? parent $root-node)))
        (string->symbol (cat (node-full-name (node-parent node))
                             "/" (node-name node)))
        (node-name node))))

  (define (clear-node-state! node)
    (set! (node-has-expensive-handlers? node) #f)
    (set! (node-handlers node) (make-hash-table))
    (set! (node-values node) (make-hash-table)))

  (define *node-table* (make-hash-table))

  (defgeneric (register-node (node <node>)))

  (defmethod (register-node (node <node>))
    (let [[name (node-full-name node)]]
      (when (hash-table-get *node-table* name (lambda () #f))
        (error (cat "Duplicate copies of node " (node-full-name node))))
      (hash-table-put! *node-table* name node)))

  (define (unregister-node node)
    ;; This is only used to delete temporary <element> nodes, simulating
    ;; end-of-card rollback.
    (let [[name (node-full-name node)]]
      (assert (and (instance-of? node <element>)
                   (element-temporary? node)))
      (assert (eq? (hash-table-get *node-table* name (lambda () #f)) node))
      (hash-table-remove! *node-table* name)))

  (define (find-node name)
    (hash-table-get *node-table* name (lambda () #f)))

  (define (find-node-relative base name)
    ;; Treat 'name' as a relative path.  If 'name' can be found relative
    ;; to 'base', return it.  If not, try the parent of base if it
    ;; exists.  If all fails, return #f.
    (if (eq? base $root-node)
        (find-node name)
        (let* [[base-name (node-full-name base)]
               [candidate (string->symbol (cat base-name "/" name))]
               [found (find-node candidate)]]
          (or found (find-node-relative (node-parent base) name)))))

  (define (@-helper name)
    (if (current-card)
        (find-node-relative (current-card) name)
        (error (cat "Can't write (@ " name ") outside of a card"))))    

  (define-syntax @
    ;; Syntactic sugar for find-node-relative.
    (syntax-rules ()
      [(@ name)
       (@-helper 'name)]))

  (define (elem-or-name-hack elem-or-name)
    ;; Backwards compatibility glue for code which refers to elements
    ;; by name.  Used by functions such as WAIT.
    (assert (or (instance-of? elem-or-name <element>)
                (instance-of? elem-or-name <symbol>)))
    (node-full-name (if (element? elem-or-name)
                        elem-or-name
                        (begin
                          (debug-caution (cat "Change '" elem-or-name
                                              " to (@ " elem-or-name ")"))
                          (find-node-relative (current-card) elem-or-name)))))

  (define (analyze-node-name name)
    ;; Given a name of the form '/', 'foo' or 'bar/baz', return the
    ;; node's parent and the "local" portion of the name (excluding the
    ;; parent).  A "/" character separates different levels of nesting.
    (if (eq? name '|/|)
        (values #f '|/|) ; Handle the root node.
        (let* [[str (symbol->string name)]
               [matches (regexp-match "^(([^/].*)/)?([^/]+)$" str)]]
          (cond
           [(not matches)
            (error (cat "Illegal node name: " name))]
           [(not (cadr matches))
            (values $root-node name)]
           [else
            (let [[parent (find-node (string->symbol (caddr matches)))]]
              (unless parent
                (error (cat "Parent of " name " does not exist.")))
              (values parent
                      (string->symbol (cadddr matches))))]))))

  (define-syntax define-node
    (syntax-rules (:template)
      [(define-node name group node-class (:template extended bindings ...)
         body ...)
       (begin
         (define name
           (with-values [parent local-name] (analyze-node-name 'name)
             (make node-class
               :group      'group
               :extends    extended
               :bindings-eval-fn (expand-bindings-eval-fn [] bindings ...)
               :init-fn    (expand-init-fn () body ...)
               :parent     parent
               :name       local-name)))
         (register-node name))]
      [(define-node name group node-class (bindings ...) body ...)
       (define-node name group node-class (:template #f bindings ...)
         body ...)]
      [(define-node name group node-class)
       (define-node name group node-class (:template #f))]))

  (define-syntax define-node-definer
    (syntax-rules ()
      [(define-node-definer definer-name group node-class)
       (define-syntax definer-name
         (syntax-rules ()
           [(definer-name name . rest)
            (define-node name group node-class . rest)]))]))

  ;;-----------------------------------------------------------------------
  ;;  Groups
  ;;-----------------------------------------------------------------------
  ;;  A group contains other nodes.

  (provide <group> group? group-children)

  (defclass <group> (<node>)
    (children :type <list> :initvalue '()))

  (defgeneric (group-add-child! (group <group>) (child <node>)))

  (defmethod (group-add-child! (group <group>) (child <node>))
    (set! (group-children group)
          (append (group-children group) (list child))))
    
  ;;-----------------------------------------------------------------------
  ;;  Jumpable
  ;;-----------------------------------------------------------------------
  ;;  The abstract superclass of all nodes which support 'jump'.  In
  ;;  general, these are either cards or <card-sequence>s.

  (provide <jumpable> jumpable?)

  (defclass <jumpable> (<node>))

  (defgeneric (jump (target <jumpable>)))

  ;;-----------------------------------------------------------------------
  ;;  Groups of Cards
  ;;-----------------------------------------------------------------------
  ;;  By default, groups of cards are not assumed to be in any particular
  ;;  linear order, at least for purposes of program navigation.

  (provide $root-node define-group-template group)

  (define (card-or-card-group? node)
    (or (card? node) (card-group? node)))

  (defclass <card-group> (<group>)
    (active? :type <boolean> :initvalue #f))

  (defgeneric (card-group-find-next (group <card-group>) (child <jumpable>)))
  (defgeneric (card-group-find-prev (group <card-group>) (child <jumpable>)))

  (defmethod (card-group-find-next (group <card-group>) (child <jumpable>))
    #f)

  (defmethod (card-group-find-prev (group <card-group>) (child <jumpable>))
    #f)

  (defmethod (group-add-child! (group <card-group>) (child <node>))
    (assert (card-or-card-group? child))
    (call-next-method))

  (define $root-node
    (make <card-group>
      :name '|/| :parent #f :active? #t))

  (define-template-definer define-group-template <card-group>)
  (define-node-definer group <card-group> <card-group>)

  ;;-----------------------------------------------------------------------
  ;;  Sequences of Cards
  ;;-----------------------------------------------------------------------
  ;;  Like groups, but ordered.

  (provide sequence)

  (defclass <card-sequence> (<jumpable> <card-group>))

  (defmethod (jump (target <card-sequence>))
    (if (null? (group-children target))
        (error (cat "Can't jump to sequence " (node-full-name target)
                    " because it contains no cards."))
        (jump (car (group-children target)))))

  (defmethod (card-group-find-next (group <card-sequence>) (child <jumpable>))
    ;; Find the node *after* child.
    (let [[remainder (memq child (group-children group))]]
      (assert (not (null? remainder)))
      (if (null? (cdr remainder))
          (card-group-find-next (node-parent group) group)
          (cadr remainder))))

  (defmethod (card-group-find-prev (group <card-sequence>) (child <jumpable>))
    ;; Find the node *before* child.
    (let search [[children (group-children group)]
                 [candidate-func 
                  (lambda ()
                    (card-group-find-prev (node-parent group) group))]]
      (assert (not (null? children)))
      (if (eq? (car children) child)
          (candidate-func)
          (search (cdr children) (lambda () (car children))))))

  (define-node-definer sequence <card-group> <card-sequence>)

  ;;-----------------------------------------------------------------------
  ;;  Cards
  ;;-----------------------------------------------------------------------
  ;;  More functions are defined in the next section below.

  (provide <card> card? card-next card-prev jump-next jump-prev
           define-card-template card)

  (defclass <card>          (<jumpable> <group>))

  (defmethod (jump (target <card>))
    (jump-to-card target))

  (define (card-next)
    (card-group-find-next (node-parent (current-card)) (current-card)))

  (define (card-prev)
    (card-group-find-prev (node-parent (current-card)) (current-card)))

  (define (jump-helper find-card str)
    (let [[c (find-card)]]
      (if c
          (jump c)
          (error (cat "No card " str " " (node-full-name (current-card))
                      " in sequence.")))))
      
  (define (jump-next) (jump-helper card-next "after"))
  (define (jump-prev) (jump-helper card-prev "before"))

  (defmethod (register-node (c <card>))
    (call-next-method)
    (%kernel-register-card c))

  (define-template-definer define-card-template <card>)
  (define-node-definer card <card> <card>)

  ;;-----------------------------------------------------------------------
  ;; Elements
  ;;-----------------------------------------------------------------------

  (provide <element> element? define-element-template element create)

  (defclass <element>       (<node>)
    (temporary? :type <boolean> :initvalue #f))

  (define-template-definer define-element-template <element>)
  (define-node-definer element <element> <element>)
  
  (define (create template &key (name (gensym)) &rest-keys bindings)
    ;; Temporarily create an <element> node belonging to the current card,
    ;; using 'template' as our template.  This node will be deleted when we
    ;; leave the card.
    ;;
    ;; Someday I expect this to be handled by a transactional rollback of the
    ;; C++ document model.  But this will simulate things well enough for
    ;; now, I think.
    (define (bindings-eval-fn self)
      (bindings->hash-table bindings))
    (assert (current-card))
    (assert (eq? (template-group template) '<element>))
    (let [[e (make <element>
               :group      '<element>
               :extends    template
               :bindings-eval-fn bindings-eval-fn
               :parent     (current-card)
               :name       name
               :temporary? #t)]]
      (register-node e)
      (enter-node e)
      e))

  (define (eq-with-gensym-hack? sym1 sym2)
    ;; STUPID BUG - We name anonymous elements with gensyms, for
    ;; which (eq? sym (string->symbol (symbol->string sym))) is never
    ;; true.  This is dumb, and is evidence of a fundamental misdesign
    ;; in element management responsibilities.  I need to fix this ASAP.
    ;; But for now, this will allow the engine to limp along.
    (equal? (symbol->string sym1) (symbol->string sym2)))

  (define (delete-element-info name)
    ;; We're called from C++ after the engine's version of the specified
    ;; element has been deleted.  Our job is to bring our data structures
    ;; back in sync.
    ;; TODO - We're called repeatedly as nodes get deleted, resulting
    ;; in an O(n^2) time to delete n nodes.  Not good, but we can live with
    ;; it for the moment.
    (let [[card (current-card)]]
      (set! (group-children card)
            (let recurse [[children (group-children card)]]
              (cond
               [(null? children) '()]
               [(eq-with-gensym-hack? name (node-full-name (car children)))
                ;; Delete this node, and exclude it from the new child list.
                (if (element-temporary? (car children))
                    (begin
                      (exit-node (car children))
                      (unregister-node (car children)))
                    (debug-caution
                     (cat "Can't fully delete non-temporary element " name
                          " in this version of the engine")))
                (recurse (cdr children))]
               [else
                ;; Keep this node.
                (cons (car children) (recurse (cdr children)))])))))
  

  ;;=======================================================================
  ;;  Changing Cards
  ;;=======================================================================

  ;; We call these function whenever we enter or exit a node.  They never
  ;; recurse up or down the node hierarchy; that work is done by other
  ;; functions below.
  (defgeneric (exit-node (node <node>)))
  (defgeneric (enter-node (node <node>)))

  (defmethod (exit-node (node <node>))
    ;; Clear our handler list.
    (clear-node-state! node))

  (defmethod (enter-node (node <node>))
    ;; TODO - Make sure all our template properties are bound.
    ;; Initialize our templates one at a time.
    (let recurse [[template node]]
      (when template
        ;; We bind property values on the way in, and run init-fns
        ;; on the way out.
        (node-bind-property-values! node template)
        (recurse (template-extends template))
        ;; Pass NODE to the init-fn so SELF refers to the right thing.
        ((template-init-fn template) node))))

  (defmethod (exit-node (group <card-group>))
    (set! (card-group-active? group) #f)
    (call-next-method))

  (defmethod (enter-node (group <card-group>))
    (call-next-method)
    (set! (card-group-active? group) #t))

  (defmethod (exit-node (card <card>))
    ;; We have some extra hooks and primitives to call here.
    (call-hook-functions *exit-card-hook* card)
    (when (have-5l-prim? 'notifyexitcard)
      (call-5l-prim 'notifyexitcard))
    (call-next-method))

  (defmethod (enter-node (card <card>))
    ;; We have some extra hooks and primitives to call here.
    (when (have-5l-prim? 'notifyentercard)
      (call-5l-prim 'notifyentercard))
    (call-hook-functions *enter-card-hook* card)
    (call-next-method)
    (call-hook-functions *card-body-finished-hook* card))

  (define (find-active-parent new-card)
    ;; Walk back up the node hierarchy from new-card until we find the
    ;; nearest active parent.  The root node is always active, so this
    ;; recursive call terminates.
    (let recurse [[group (node-parent new-card)]]
      (if (card-group-active? group)
          group
          (recurse (node-parent group)))))

  (define (exit-card-group-recursively group stop-before)
    ;; Recursively exit nested card groups starting with 'group', but
    ;; ending before 'stop-before'.
    (let recurse [[group group]]
      (unless (eq? group stop-before)
        (assert (node-parent group))
        (exit-node group)
        (recurse (node-parent group)))))

  (define (enter-card-group-recursively group)
    ;; Recursively enter nested card groups until we have a chain of
    ;; active groups from the root to 'group'.
    (let recurse  [[group group]]
      (unless (card-group-active? group)
        (assert (node-parent group))
        (recurse (node-parent group))
        (enter-node group))))
  
  (define (delete-element elem)
    ;; A little placeholder to make deletion work the same way in Tamale
    ;; and in Common test.
    ;; TODO - Remove when cleaning up element deletion.
    (if (have-5l-prim? 'deleteelements)
        (call-5l-prim 'deleteelements (node-full-name elem))
        (delete-element-info (node-full-name elem))))

  (define (exit-card old-card new-card)
    ;; Exit all our child elements.
    ;; TRICKY - We call into the engine to do element deletion safely.
    ;; We work with a copy of (GROUP-CHILDREN OLD-CARD) the original
    ;; will be modified as we run.
    (foreach [child (group-children old-card)]
      (delete-element child))
    ;; Exit old-card.
    (exit-node old-card)
    ;; Exit as many enclosing card groups as necessary.
    (exit-card-group-recursively (node-parent old-card)
                                 (find-active-parent new-card)))

  (define (enter-card new-card)
    ;; Clear our handler list.  We also do this in exit-node; this
    ;; invocation is basically defensive, in case something went
    ;; wrong during the node exiting process.
    (clear-node-state! new-card)
    ;; Enter as many enclosing card groups as necessary.
    (enter-card-group-recursively (node-parent new-card))
    ;; Enter all our child elements.  Notice we do this first, so
    ;; all the elements are available by the time we run the card body.
    ;; Unfortunately, this means that "new element" events generated
    ;; during element initialization can't be caught by the card.
    ;; Weird, but this is what the users voted for.
    (foreach [child (group-children new-card)]
      (enter-node child))
    ;; Enter new-card.
    (enter-node new-card))

  (define (maybe-enable-expensive-events-for-card card)
    ;; REGISTER-EVENT-HANDLER attempts to turn on expensive events whenever
    ;; a matching handler is installed.  But we need to reset the
    ;; expensive event state when changing cards.  This means we need
    ;; to pay close attention to any nodes which live longer than a card.
    (let [[enable? #f]]
      (let recurse [[node card]]
        (when node
          (if (node-has-expensive-handlers? node)
              (set! enable? #t)
              (recurse (node-parent node)))))
      (enable-expensive-events enable?)))

  (define (run-card card)
    (%kernel-clear-timeout)

    ;; Finish exiting our previous card.
    (when *%kernel-current-card*
      (exit-card *%kernel-current-card* card))

    ;; Reset the origin to 0,0.
    (call-5l-prim 'resetorigin)

    ;; Update our global variables.
    (set! *%kernel-previous-card* *%kernel-current-card*)
    (set! *%kernel-current-card* card)

    ;; Update our expensive event state.
    (maybe-enable-expensive-events-for-card card)
    
    ;; Actually run the card.
    (debug-log (cat "Begin card: <" (%kernel-card-name card) ">"))
    (with-errors-blocked (non-fatal-error)
      (enter-card card)
      (refresh)))


  ;;=======================================================================
  ;;  Cards
  ;;=======================================================================
  ;;  Older support code for cards.  Some of this should probably be
  ;;  refactored to an appropriate place above.

  (provide card-exists? find-card current-card card-name)

  ;; TODO - A different meaning of "previous" from the one above.  Rename.
  (define *%kernel-current-card* #f)
  (define *%kernel-previous-card* #f)
  
  ;; TODO - This glue makes <card> look like the old %kernel-card.  Remove.
  ;; TODO - We need to start creating sequences, etc., to contain cards.
  ;;(define-struct %kernel-card (name thunk) (make-inspector))
  (define %kernel-card? card?)
  (define %kernel-card-name node-full-name)
  (define %kernel-card-thunk template-init-fn)

  ;; TODO - Redundant with *node-table*.  Remove.
  (define *%kernel-card-table* (make-hash-table))
  
  (define (%kernel-register-card card)
    (let ((name (%kernel-card-name card)))
      (if (hash-table-get *%kernel-card-table* name (lambda () #f))
          (non-fatal-error (cat "Duplicate card: " name))
          (begin
            (if (have-5l-prim? 'RegisterCard)
                (call-5l-prim 'RegisterCard name))
            (hash-table-put! *%kernel-card-table* name card)))))

  (define (card-exists? card-name)
    (if (hash-table-get *%kernel-card-table* (string->symbol card-name)
                        (lambda () #f))
        #t
        #f))

  (define (find-card card-or-name)
    (cond
     [(%kernel-card? card-or-name)
      card-or-name]
     [(symbol? card-or-name)
      (let ((card (hash-table-get *%kernel-card-table*
                                  card-or-name
                                  (lambda () #f))))
        (or card (throw (cat "Unknown card: " card-or-name))))]
     [(string? card-or-name)
      (find-card (string->symbol card-or-name))]
     [#t
      (throw (cat "Not a card: " card-or-name))]))

  (define (current-card)
    *%kernel-current-card*)    

  (define (card-name card-or-name)
    (cond
     [(%kernel-card? card-or-name)
      (%kernel-card-name card-or-name)]
     [(symbol? card-or-name)
      (symbol->string card-or-name)]
     [(string? card-or-name)
      card-or-name]
     [#t
      (throw (cat "Not a card: " card-or-name))]))

  ) ; end module
