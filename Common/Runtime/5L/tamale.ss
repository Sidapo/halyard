;;=========================================================================
;; Random Tamale Primitives
;;=========================================================================
;; This is a collection of loosely documented and poorly-organized Tamale
;; primitives, all subject to change at a moment's notice.

(module tamale (lib "5l.ss" "5L")

  (provide load-picture set-image-cache-size! modal-input
           zone set-zone-cursor! register-cursor mouse-position
           delete-element delete-elements
           clear-screen rect-horizontal-center rect-vertical-center
           rect-center move-rect-left-to move-rect-top-to
           move-rect-horizontal-center-to move-rect-vertical-center-to
           move-rect-center-to center-text html edit-box movie pause resume
           wait tc draw-line draw-box draw-box-outline inset-rect timeout
           current-card-name fade unfade)

  (define (make-path subdir path)
    (apply build-path (current-directory) subdir (regexp-split "/" path)))

  (define (load-picture name p &key (subrect :rect #f))
    (let [[path (make-path "Graphics" name)]]
      (unless (file-exists? path)
        (throw (cat "No such graphic: " path)))
      (if subrect
          (call-5l-prim 'loadsubpic path p subrect)
          (call-5l-prim 'loadpic path p))))
  
  (define (set-image-cache-size! bytes)
    (call-5l-prim 'SetImageCacheSize bytes))

  (define (modal-input r size forecolor backcolor)
    (call-5l-prim 'input r size forecolor backcolor)
    (engine-var '_modal_input_text))
  
  (define (zone name r action &key (cursor 'hand))
    (call-5l-prim 'zone name r action cursor))
  
  (define (set-zone-cursor! name cursor)
    (call-5l-prim 'SetZoneCursor name cursor))

  (define (register-cursor sym filename &key (hotspot (point -1 -1)))
    (let [[path (make-path "Graphics" (cat "cursors/" filename))]]
      (unless (file-exists? path)
        (throw (cat "No such cursor: " path)))
      (call-5l-prim 'RegisterCursor sym path hotspot)))

  (define (mouse-position)
    ;; XXX - AYIEEEE!  We can't actually return points from the engine, so
    ;; we need to use this vile, disgusting hack instead.  Let's fix the
    ;; data model at the interpreter/engine boundary, OK?
    ;;
    ;; XXX - This keeps returning exciting results even if we're in the
    ;; background.  Yuck.
    (let* [[str (call-5l-prim 'MousePosition)]
           [lst (map string->number (regexp-split " " str))]]
      (point (car lst) (cadr lst))))

  (define (delete-element name)
    (delete-elements (list name)))
  
  (define (delete-elements &opt (names '()))
    (apply call-5l-prim 'deleteelements names))
  
  (define (clear-screen c)
    (call-5l-prim 'screen c))
  
  (define (rect-horizontal-center r)
    (+ (rect-left r) (round (/ (- (rect-right r) (rect-left r)) 2))))
  
  (define (rect-vertical-center r)
    (+ (rect-top r) (round (/ (- (rect-bottom r) (rect-top r)) 2))))
  
  (define (rect-center r)
    (point (rect-horizontal-center r) (rect-vertical-center r)))
  
  (define (move-rect-left-to r h)
    (rect h (rect-top r) (+ h (rect-width r)) (rect-bottom r)))

  (define (move-rect-top-to r v)
    (rect (rect-left r) v (rect-right r) (+ v (rect-height r))))

  (define (move-rect-horizontal-center-to r x)
    (move-rect-left-to r (- x (round (/ (rect-width r) 2)))))

  (define (move-rect-vertical-center-to r y)
    (move-rect-top-to r (- y (round (/ (rect-height r) 2)))))

  (define (move-rect-center-to r p)
    (move-rect-horizontal-center-to (move-rect-vertical-center-to r
                                                                  (point-y p))
                                    (point-x p)))

  (define (center-text stylesheet box msg &key (axis 'both))
    (define bounds (measure-text stylesheet msg :max-width (rect-width box)))
    (define r
      (case axis
        [[both]
         (move-rect-center-to bounds (rect-center box))]
        [[y]
         (move-rect-left-to
          (move-rect-vertical-center-to bounds (rect-vertical-center box))
          (rect-left box))]
        [[x]
         (move-rect-top-to
          (move-rect-horizontal-center-to bounds (rect-horizontal-center box))
          (rect-top box))]
        [else
         (throw (cat "center-text: Unknown centering axis: " axis))]))
    (draw-text stylesheet r msg))

  (define (html name r location)
    (call-5l-prim 'html name r (build-path (current-directory) location)))

  (define (edit-box name r text)
    (call-5l-prim 'editbox name r text))
  
  (define (movie name r location
                 &key controller? audio-only? loop? interaction?)
    (let [[path (make-path "Media" location)]]
      (unless (file-exists? path)
        (throw (cat "No such movie: " path)))
      (call-5l-prim 'movie name r
                    path
                    controller? audio-only? loop? interaction?)))

  ;; Note: these functions may not be happy if the underlying movie code
  ;; doesn't like to be paused.
  (define (pause name)
    (call-5l-prim 'pause name))

  (define (resume name)
    (call-5l-prim 'resume name))
  
  (define (wait name &key frame)
    (if frame
        (call-5l-prim 'wait name frame)
        (call-5l-prim 'wait name)))
  
  (define (tc arg1 &opt arg2 arg3)
    (cond
     [arg3 (+ (* (+ (* arg1 60) arg2) 30) arg3)]
     [arg2 (+ (* arg1 30) arg2)]
     [else arg1]))
  
  (define (draw-line from to c width)
    (call-5l-prim 'drawline from to c width))

  (define (draw-box r c)
    (call-5l-prim 'drawboxfill r c))

  (define (draw-box-outline r c width)
    (call-5l-prim 'drawboxoutline r c width))
  
  (define (inset-rect r pixels)
    ;; TODO - Rename foo-offset to offset-foo.
    (rect (+ (rect-left r) pixels)
          (+ (rect-top r) pixels)
          (- (rect-right r) pixels)
          (- (rect-bottom r) pixels)))
  
  (define (timeout seconds card)
    (call-5l-prim 'timeout seconds (card-name card)))

  (define (current-card-name)
    (card-name (current-card)))

  (define (fade)
    (call-5l-prim 'fade))

  (define (unfade)
    (call-5l-prim 'unfade))

  )