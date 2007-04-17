(module animate (lib "5l.ss" "5L")

  ;;=======================================================================
  ;;  Utility routines
  ;;=======================================================================
  ;;  The names and details of these functions are extremely likely to
  ;;  change.

  ;; Helpful functions for listing the frames of an animation.
  (provide range strings zero-pad)

  ;; Return a list of numbers from START to END, inclusive, increasing by
  ;; STEP.
  (define (range start end &opt (step 1))
    (define (range-internal start)
      (cond 
        ((> start end) '())
        ((= start end) (list end))
        (else (cons start (range-internal (+ start step))))))
    (range-internal start))

  (define (strings prefix lst suffix)
    (map (fn (x) (cat prefix x suffix))
         lst))

  (define (zero-pad n x)
    (define str (cat x))
    (cat (make-string (- n (string-length str)) #\0) str))
  
  
  ;;;======================================================================
  ;;;  General Animation Support
  ;;;======================================================================
  ;;;  TODO - Some API details, especially names, may change slightly.

  (provide slide reshape transform play
           simultaneously after animate interpolate ease-in ease-out
           ease-in/out do-nothing)

  ;;; Given x(0), x(1), dx/dt(0), and dx/dt(1), fit a cubic polynomial to
  ;;; those four constraints and evaluate it.
  (define (eval-cubic-1d-spline x0 x1 dx/dt0 dx/dt1 t)
    ;; The equation for this spline was determined using the following
    ;; commands in Maxima, a GPL'd version of MacSyma.  (Below, xp(t) is
    ;; dx/dt at t.)
    ;;
    ;;   x(t) := a*t^3 + b*t^2 + c*t + d;
    ;;   define(xp(t), diff(x(t),t));
    ;;   solve([x(0)=x0,x(1)=x1,xp(0)=xp0,xp(1)=xp1],[a,b,c,d]);
    ;;
    ;; Maxima reported the following result:
    ;;
    ;;   a = xp1 + xp0 - 2 x1 + 2 x0
    ;;   b = - xp1 - 2 xp0 + 3 x1 - 3 x0
    ;;   c = xp0
    ;;   d = x0
    (let [[a (+ (* 2 x0) (* -2 x1) dx/dt0 dx/dt1)]
          [b (+ (* -3 x0) (* 3 x1) (* -2 dx/dt0) (* -1 dx/dt1))]
          [c dx/dt0]
          [d x0]]
      (+ (* a (expt t 3)) (* b (expt t 2)) (* c t) d)))

  (define (ease-fn x0 x1 dx/dt0 dx/dt1)
    (fn (anim-thunk)
      (callback
        (define draw-func (anim-thunk))
        (fn (fraction)
          (draw-func (eval-cubic-1d-spline x0 x1 dx/dt0 dx/dt1 fraction))))))

  (define ease-in (ease-fn 0 1 0 1))
  (define ease-out (ease-fn 0 1 1 0))
  (define ease-in/out (ease-fn 0 1 0 0))

  ;;; Creates an animator that interpolates a given expression (which can 
  ;;; be read and SET!) from its initial value.
  ;;;
  ;;; XXX - Handle nested "places" of the form (interpolate (f1 (f2 x))
  ;;; final) in a more graceful fashion.
  (define-syntax interpolate
    (syntax-rules ()
      [(_ var final) 
       ;; Wrapped in a let so the samantics match that of all other
       ;; animators, which is that the FINAL value is evaluated at the
       ;; time the animator is constructed, since they are just
       ;; functions.
       (let [[end final]]
         (callback
           (define start var)
           (fn (fraction)
             (set! var (interpolate-value fraction start end)))))]))
  
  ;;; This function creates a simple DRAW-FUNC for use with ANIMATE.  The
  ;;; resulting function moves OBJs from the point FROM to the point TO
  ;;; over the duration of the animation.
  (define (slide elem to)
    (interpolate (prop elem at) to))

  ;;; Return an animator that resizes an element to a given rect.  Mostly 
  ;;; useful for changing the size of a rectangle or rectangle-outline element.
  (define (reshape elem final-rect)
    (interpolate (prop elem shape) final-rect))

  ;;; Returns an animator that plays through the frames of a %SPRITE%.
  (define (play sprite &key (reverse? #f))
    (interpolate (prop sprite frame) 
                 (if reverse? 0 (- (length (prop sprite frames)) 1))))

  ;;; Return an animator that changes the rect for an element to a given rect 
  ;;; (both shape and position).
  (define (transform elem new-rect)
    (simultaneously 
      (slide elem (shape-origin new-rect))
      (reshape elem (move-shape-left-top-to new-rect (point 0 0)))))

  ;;; Create a function that will invoke multiple animations simultaneously
  ;;; (when it is called).
  (define (simultaneously &rest anim-thunks)
    (callback 
      (define draw-funcs (map (fn (x) (x)) anim-thunks))
      (fn (fraction)
        (foreach [f draw-funcs]
          (f fraction)))))

  (define (do-nothing)
    (callback
      (fn (fraction)
        #f)))

  ;;; Play animations in sequence, each one starting at a given fraction.
  (define (after &rest args)
    ;; A whole bunch of helper functions for implementing AFTER. All
    ;; of he helper functions that don't close over the state of the
    ;; animation are defined here; the ones that do close over the
    ;; state are defined in the animation thunk.
    (define (add-zero lst)
      (if (= (car lst) 0.0)
        lst
        (append (list 0.0 (do-nothing)) lst)))
    (define (pad-end lst)
      (if (odd? (length lst))
        (append lst (list (do-nothing)))
        lst))
    (define (split lst)
      (let loop [[result (cons '() '())] [lst lst]]
        (cond
         [(null? lst) (cons (reverse (car result)) (reverse (cdr result)))]
         [(null? (cdr lst)) (error (cat "Bad argument list: " args))]
         [else (loop (cons (cons (car lst) (car result)) 
                           (cons (cadr lst) (cdr result)))
                     (cddr lst))])))
    ;; TODO - this is confusing, comment it.
    (define (index-for-time start-times time)
      (let loop [[c 0] [times start-times]]
        (cond
         [(or (null? times) (null? (cdr times))) 
          (error "Bad arguments passed to index-for-time" start-times time)]
         [(and (null? (cddr times)) (<= (car times) time (cadr times)))
          c]
         [(and (<= (car times) time) (< time (cadr times))) c]
         [else (loop (+ c 1) (cdr times))])))
    (define (scaled-time start end time)
      (assert (<= start time end))
      (/ (- time start) (- end start)))
    (callback
      (define last-index 0)
      (define anim-fns '())
      (define split-list (split (pad-end (add-zero args))))
      ;; We add an extra 1.0 to the times list to make all of the time
      ;; range calculations easier (so they can just treat this 1.0 as
      ;; the end of the range, instead of having a special case).
      (define times (append (car split-list) (list 1.0)))
      (define thunks (cdr split-list))
      ;; This function returns the animation function for a given
      ;; index, evaluating its thunk if it hasn't already been
      ;; evaluated. 
      ;; NOTE: due to our condition that in an AFTER clause, each
      ;; animation thunk is only evaluated after all previous
      ;; animations have completed, it is incorrect to call this
      ;; function for a given index before all previous animation
      ;; functions have been completed (called with argument 1.0).
      (define (anim-fn-for-index index)
        (if (< index (length anim-fns))
          (list-ref anim-fns index)
          (begin 
            (assert (= index (length anim-fns)))
            (let [[anim-fn ((list-ref thunks index))]]
              (set! anim-fns (append anim-fns (list anim-fn)))
              anim-fn))))
      (fn (fraction) 
        (define index (index-for-time times fraction))
        (let loop [[n last-index]]
          (if (= n index)
            ((anim-fn-for-index n) 
             (scaled-time (list-ref times index) 
                          (list-ref times (+ index 1))
                          fraction))
            (begin ((anim-fn-for-index n) 1.0)
                   (loop (+ n 1)))))
        (set! last-index index))))

  ;;; Call DRAW-FUNC with numbers from 0.0 to 1.0 over the course
  ;;; MILLISECONDS.
  (define (animate milliseconds &rest anim-thunks)
    (define start-time (current-milliseconds))
    (define end-time (+ start-time milliseconds))
    (define draw-func ((apply simultaneously anim-thunks)))
    (draw-func 0.0)
    (let loop []
      (let [[current-time (current-milliseconds)]]
        (when (< current-time end-time)
          (let* [[elapsed-time (- current-time start-time)]
                 [fraction (/ (* 1.0 elapsed-time) milliseconds)]]
            (draw-func fraction)
            (idle)
            (loop)))))
    (draw-func 1.0))
  
  )
