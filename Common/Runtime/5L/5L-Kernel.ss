(module 5L-Kernel (lib "lispish.ss" "5L")

  ;; Import %call-5l-prim from the engine.
  (require #%fivel-engine)

  ;; Get begin/var.
  (require (lib "begin-var.ss" "5L"))
  (provide begin/var)
  
  ;; Get hooks, and re-export them.
  (require (lib "hook.ss" "5L"))
  (provide (all-from (lib "hook.ss" "5L")))


  ;;=======================================================================
  ;;  Built-in Types
  ;;=======================================================================
  ;;  These methods implement various "built-in" types that are known to
  ;;  the 5L engine.  They should *never* raise errors, because they're
  ;;  called directly from C++ code that isn't prepared to cope with Scheme
  ;;  errors.

  (provide (rename point <point>) (rename make-point point) point?
	   point-x set-point-x! point-y set-point-y!

	   (rename rect <rect>) (rename make-rect rect) rect?
	   rect-left set-rect-left! rect-top set-rect-top!
	   rect-right set-rect-right! rect-bottom set-rect-bottom!

	   (rename color <color>) (rename make-color-opt-alpha color) color?
	   color-red set-color-red! color-green set-color-green!
	   color-blue set-color-blue! color-alpha set-color-alpha!

	   (rename percent <percent>) (rename make-percent percent)
	   percent? percent-value)
  
  (define-struct point (x y) (make-inspector))

  (define-struct rect (left top right bottom) (make-inspector))

  (define-struct color (red green blue alpha) (make-inspector))

  (define (make-color-opt-alpha r g b &opt (a 0))
    (make-color r g b a))

  (define-struct percent (value) (make-inspector))
  

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

  (provide member? value->string cat label with-errors-blocked)
  
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

  (define-syntax label
    (syntax-rules ()
      [(label name body ...)
       (call-with-current-continuation (lambda (name)
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
  ;;  Internal State
  ;;=======================================================================
  ;;  When the interpreter returns from a primitive call (including a
  ;;  primitive call to run the idle loop), it can be in one of a number of
  ;;  states:
  ;;
  ;;    NORMAL:   The interpreter is running code normally, or has no code
  ;;              to run.  The default.
  ;;    PAUSED:   The interpreter should pause the current card until it is
  ;;              told to wake back up.
  ;;    JUMPING:  The interpreter should execute a jump.
  ;;    NAPPING:  The interpreter should pause the current card for the
  ;;              specified number of milliseconds.
  ;;    CARD-KILLED: The interpreter should stop executing the current
  ;;               card, and return to the top-level loop.
  ;;    INTERPRETER-KILLED: The interpreter should exit.
  ;;
  ;;  Callbacks are slightly special, however--see the code for details.
  
  (provide call-at-safe-time)

  (define *%kernel-running-callback?* #f)
  (define *%kernel-state* 'NORMAL)
  
  (define *%kernel-jump-card* #f)
  (define *%kernel-timeout* #f)
  (define *%kernel-timeout-thunk* #f)
  (define *%kernel-nap-time* #f)

  (define *%kernel-running-deferred-thunks?* #f)
  (define *%kernel-deferred-thunk-queue* '())
  
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
    (when (and *%kernel-timeout*
	       (>= (current-milliseconds) *%kernel-timeout*))
      (let ((thunk *%kernel-timeout-thunk*))
	(%kernel-clear-timeout)
	(thunk))))

  (define (%kernel-safe-to-run-deferred-thunks?)
    ;; Would now be a good time to run deferred thunks?  Wait until
    ;; nothing exciting is happening.
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
    ;; Run any deferred thunks.
    (unless (or (null? *%kernel-deferred-thunk-queue*)
		(not (%kernel-safe-to-run-deferred-thunks?)))

      ;; Make a copy of the old queue and clear the global variable
      ;; (which may be updated behind our backs.
      (let [[items (reverse *%kernel-deferred-thunk-queue*)]]
	(set! *%kernel-deferred-thunk-queue* '())

	;; Run every thunk in the queue, in order.
	(fluid-let [[*%kernel-running-deferred-thunks?* #t]]
	  (let loop [[items items]]
	    (unless (null? items)
	      ((car items))
	      (loop (cdr items))))))

      ;; Check to see if any new items appeared in the queue while we
      ;; were running the first batch.
      (%kernel-check-deferred)))

  (define (%kernel-clear-state)
    (set! *%kernel-state* 'NORMAL)
    (set! *%kernel-jump-card* #f))
  
  (define (%kernel-run-as-callback thunk)
    (assert (not *%kernel-running-callback?*))
    (if (eq? *%kernel-state* 'INTERPRETER-KILLED)
	(5l-log "Skipping callback because interpreter is being shut down")
	(let [[saved-kernel-state *%kernel-state*]]
	  (set! *%kernel-state* 'NORMAL)
	  (label exit-callback
	    ;; TODO - Can we have better error handling?
            (with-errors-blocked (non-fatal-error)
	      (fluid-let [[*%kernel-exit-to-top-func* exit-callback]
			  [*%kernel-exit-interpreter-func* exit-callback]
			  [*%kernel-running-callback?* #t]]
		(thunk))))
	  (if (eq? *%kernel-state* 'NORMAL)
	      (set! *%kernel-state* saved-kernel-state)))))

  (define (%kernel-set-state state)
    (set! *%kernel-state* state))

  (define (%kernel-check-state)
    (%kernel-check-deferred) ; Should be the first thing we do.
    (unless *%kernel-running-callback?*
      (%kernel-check-timeout))
    (case *%kernel-state*
      [[NORMAL]
       #f]
      [[PAUSED]
       (%call-5l-prim 'schemeidle)
       (%kernel-check-state)]
      [[NAPPING]
       (if (< (current-milliseconds) *%kernel-nap-time*)
	   (begin
	     (%call-5l-prim 'schemeidle)
	     (%kernel-check-state))
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
  
  (provide call-5l-prim have-5l-prim? idle 5l-log debug-log
	   caution debug-caution non-fatal-error fatal-error
	   engine-var set-engine-var! throw exit-script jump refresh)

  (define (call-5l-prim type . args)
    (let ((result (apply %call-5l-prim args)))
      (%kernel-check-state)
      (coerce-from-5l-type type result)))
  
  (define (have-5l-prim? name)
    ;; TODO - Build a general mechanism for extracting return values
    ;; and converting them to the correct type.
    (equal? (%call-5l-prim 'haveprimitive name) "1"))

  (define (idle)
    (%kernel-die-if-callback 'idle)
    (call-5l-prim 'VOID 'schemeidle))
  
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
  
  (define (coerce-from-5l-type type val)
    (case type
      [[VOID] #f]
      [[INTEGER] (string->number val)]
      [[STRING] val]
      [[BOOL]
       (cond
	[(equal? val "0") #f]
	[(equal? val "1") #t]
	[else
	 (throw (cat "Cannot coerce " val " to boolean value"))])]
      [else
       (throw (cat "Unknown engine variable type " type " for " val))]))

  (define (coerce-to-5l-type type val)
    (case type
      [[INTEGER]
       (assert (and (integer? val) (exact? val)))
       (number->string val)]
      [[STRING]
       (assert (string? val))
       val]
      [[BOOL]
       (if val #t #f)]
      [else
       (throw (cat "Unknown engine variable type " type " for " val))]))

  (define (engine-var name type)
    (call-5l-prim type 'get name))
  
  (define (set-engine-var! name type value)
    (call-5l-prim 'VOID 'set name (coerce-to-5l-type type value)))
  
  (define (throw msg)
    ;; TODO - More elaborate error support.
    (non-fatal-error msg)
    (error msg))
  
  (define (exit-script)
    (call-5l-prim 'VOID 'schemeexit))
  
  (define (jump card)
    (if (have-5l-prim? 'jump)
	(call-5l-prim 'VOID 'jump (card-name card))
	(begin
	  ;; If we don't have a JUMP primitive, fake it by hand.
	  (set! *%kernel-jump-card* (%kernel-find-card card))
	  (%kernel-set-state 'JUMPING)
	  (%kernel-check-state))))

  (define (refresh)
    (call-hook-functions *before-draw-hook*)
    (if (have-5l-prim? 'unlock)
	(call-5l-prim 'VOID 'unlock)))


  ;;=======================================================================
  ;;  Cards
  ;;=======================================================================

  (provide card-exists? card-name card)

  (define *%kernel-current-card* #f)
  (define *%kernel-previous-card* #f)
  
  (define-struct %kernel-card (name thunk) (make-inspector))
  
  (define *%kernel-card-table* (make-hash-table))
  
  (define (%kernel-register-card card)
    (let ((name (%kernel-card-name card)))
      (if (hash-table-get *%kernel-card-table* name (lambda () #f))
	  (non-fatal-error (cat "Duplicate card: " name))
	  (hash-table-put! *%kernel-card-table* name card))))

  (define (card-exists? card-name)
    (if (hash-table-get *%kernel-card-table* (string->symbol card-name)
			(lambda () #f))
	#t
	#f))
  
  (define (%kernel-run-card card)
    (%kernel-clear-timeout)

    ;; Finish exiting our previous card.
    (when *%kernel-current-card*
      (call-hook-functions *exit-card-hook* *%kernel-current-card*))

    ;; Update our global variables.
    (set! *%kernel-previous-card* *%kernel-current-card*)
    (set! *%kernel-current-card* card)

    ;; Actually run the card.
    (debug-log (cat "Begin card: <" (%kernel-card-name card) ">"))
    (call-hook-functions *enter-card-hook* *%kernel-current-card*)
    (with-errors-blocked (non-fatal-error)
      (call-5l-prim 'VOID 'resetorigin)
      ((%kernel-card-thunk card))
      (call-hook-functions *card-body-finished-hook* card)
      (refresh)))

  (define (%kernel-find-card card-or-name)
    (cond
     [(%kernel-card? card-or-name)
      card-or-name]
     [(symbol? card-or-name)
      (let ((card (hash-table-get *%kernel-card-table*
				  card-or-name
				  (lambda () #f))))
	(or card (throw (cat "Unknown card: " card-or-name))))]
     [(string? card-or-name)
      (%kernel-find-card (string->symbol card-or-name))]
     [#t
      (throw (cat "Not a card: " card-or-name))]))

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

  (define-syntax card
    (syntax-rules ()
      [(card name body ...)
       (begin
	 (define name (make-%kernel-card 'name
                                         (lambda () (begin/var body ...))))
	 (%kernel-register-card name))]))


  ;;=======================================================================
  ;;  Kernel Entry Points
  ;;=======================================================================
  ;;  The '%kernel-' methods are called directly by the 5L engine.  They
  ;;  shouldn't raise errors, because they're called directly from C++ code
  ;;  that doesn't want to catch them (and will, in fact, quit the
  ;;  program).

  (define (%kernel-run)
    (with-errors-blocked (fatal-error)
      (label exit-interpreter
        (fluid-let ((*%kernel-exit-interpreter-func* exit-interpreter))
	  (let ((jump-card #f))
	    (let loop ()
	      (label exit-to-top
	        (fluid-let ((*%kernel-exit-to-top-func* exit-to-top))
		  (idle)
		  (cond
		   [jump-card
		    (%kernel-run-card (%kernel-find-card jump-card))
		    (set! jump-card #f)]
		   [#t])))
	      (when (eq? *%kernel-state* 'JUMPING)
	        (set! jump-card *%kernel-jump-card*))
	      (%kernel-clear-state)
	      (loop)))))
      (%kernel-clear-state)
      (%kernel-clear-timeout)))

  (define (%kernel-kill-interpreter)
    (%kernel-set-state 'INTERPRETER-KILLED))
  
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
			 (lambda () (jump card-name))))

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

  (define (%kernel-run-callback thunk)
    (%kernel-die-if-callback '%kernel-run-callback)
    (%kernel-run-as-callback thunk))

  ) ; end module
