;; @BEGIN_LICENSE
;;
;; Halyard - Multimedia authoring and playback system
;; Copyright 1993-2008 Trustees of Dartmouth College
;; 
;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU Lesser General Public License as
;; published by the Free Software Foundation; either version 2.1 of the
;; License, or (at your option) any later version.
;; 
;; This program is distributed in the hope that it will be useful, but
;; WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;; Lesser General Public License for more details.
;; 
;; You should have received a copy of the GNU Lesser General Public
;; License along with this program; if not, write to the Free Software
;; Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
;; USA.
;;
;; @END_LICENSE

(module halyard-unit (lib "halyard.ss" "halyard")
  (require (lib "mizzen-unit.ss" "mizzen"))
  (provide (all-from (lib "mizzen-unit.ss" "mizzen")))

  (provide %test-suite% $halyard-unit-style run-all-test-suites)
  
  (define-stylesheet $halyard-unit-style
    :family "Nimbus Sans L"
    :size 16
    :color (color 0 0 0)
    :highlight-color (color #x00 #x00 #xC0))
  
  (define-stylesheet $halyard-unit-title-style
    :base $halyard-unit-style
    :size 24)
  
  (define-stylesheet $halyard-unit-passed-style
    :base $halyard-unit-style
    :size 36
    :flags '(bold)
    :color (color #x00 #xC0 #x00))
  
  (define-stylesheet $halyard-unit-failed-style 
    :base $halyard-unit-passed-style
    :color (color #xC0 #x00 #x00))
  
  ;; Are we current attempting to run through all the test cards
  ;; automatically?
  (define *running-all-tests?* #f)

  ;; An optional thunk to run when we're done with the tests.  Takes one
  ;; argument, a boolean indication whether the tests succeeded.
  (define *run-when-done-with-tests* #f)

  ;; Called after we 
  (define (done-with-tests success?)
    (set! *running-all-tests?* #f)
    (when *run-when-done-with-tests*
      (*run-when-done-with-tests* success?)
      (set! *run-when-done-with-tests* #f)))

  ;;; When called, this function will run all the test-suite cards in the
  ;;; group "tests".
  (define (run-all-test-suites &opt run-when-done-with-tests)
    (set! *run-when-done-with-tests* run-when-done-with-tests)
    (define first-test (tests/run-all .card-next))
    (if first-test
      (begin
        (set! *running-all-tests?* #t)
        (jump (first-test .card-next)))
      (done-with-tests #t)))

  ;;; Display the results of a set of tests on a card.
  (define-class %test-suite% (%card%)
    (attr tests)

    (def (setup)
      (super)
      (clear-dc (color #xFF #xFF #xFF))
      ;; Draw a title on the card (making it easier to tell when each
      ;; test-suite card is loaded).
      (draw-text (rect 30 30 800 100) $halyard-unit-title-style
                 (cat "Card: " (.full-name))))

    (def (run)
      (super)
      (let [[report (%test-report% .new)]]
        (foreach [test-class (.tests)]
          (test-class .run-tests report))
        (.report-test-results report)))

    (def (report-test-results report)
      (define (draw-result style text)
      (draw-text (rect 100 100 700 175) style text))
      (if (report .success?)
        (begin
          (draw-result $halyard-unit-passed-style "OK")
          (when *running-all-tests?*
            (let [[next (card-next)]]
              (if next
                (jump next)
                (done-with-tests #t)))))
        (begin
            (draw-result $halyard-unit-failed-style "FAILED")
            (draw-text (rect 100 175 700 500) $halyard-unit-style
                       (apply string-append
                              (map (fn (failure)
                                     (string-append
                                      "<h><b>" (string->xml 
                                                (failure .title))
                                      "</b></h>\n"
                                      (string->xml 
                                       (failure .message))
                                      "\n\n"))
                                   (report .failures))))
            (done-with-tests #f))))
    )

  ;;========================================================================
  ;;  Test fixtures
  ;;========================================================================
  ;;  We support "fixture" directories, which contain files used by various
  ;;  test suites.

  (provide halyard-fixture-dir)
  (define (halyard-fixture-dir name)
    (build-path (current-directory) "Runtime" "halyard" (cat name "-fixtures")))


  ;;========================================================================
  ;;  Standard test sequence
  ;;========================================================================
  ;;  Cards containing unit tests should all be placed into this sequence.

  (require (lib "tests.ss" "mizzen"))

  (provide tests)

  (group tests (%card-group% :ordered? #t))

  (card tests/run-all (%card%)
    (text instructions ((point 0 0) $halyard-unit-title-style
                        "Click to run tests")
      (setup
        (.center-on-parent!)))
    (clickable-zone run-zone ($screen-rect run-all-test-suites))
        
    (setup
      (clear-dc (color #xFF #xFF #xFF)))
    )

  (card tests/mizzen (%test-suite% :tests $mizzen-tests))


  ;;========================================================================
  ;;  Command-line test driver
  ;;========================================================================
  ;;  A slightly hackish tool for running all of a script's test suites from
  ;;  the command line.

  (provide command-line-test-driver)

  (define *activate-command-line-test-driver?* #f)

  ;;; To automatically load a script and run all its test suites, run:
  ;;;
  ;;;   Halyard_d.exe -e "(command-line-test-driver)" .
  (define (command-line-test-driver)
    (set! *activate-command-line-test-driver?* #t))

  (define (exit-on-success success?)
    (when success?
      (exit-script)))

  (with-instance %card%
    ;; Instead of running the first card that we load, instead jump
    ;; immediately to our test suites.  Yes, this is a bit of a hack, but
    ;; it's the easiest way to hook into the startup process.
    (advise before (run)
      (when *activate-command-line-test-driver?*
        (set! *activate-command-line-test-driver?* #f)
        (run-all-test-suites exit-on-success))))
  
  )
