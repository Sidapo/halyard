(module tamale-unit (lib "5L.ss" "5L")
  (provide %test-suite%)
  
  (define-stylesheet $tamale-unit-style
    :family "Nimbus Sans L"
    :size 16
    :color (color 0 0 0)
    :highlight-color (color #x00 #x00 #xC0))
  
  (define-stylesheet $tamale-unit-passed-style
    :base $tamale-unit-style
    :size 36
    :flags '(bold)
    :color (color #x00 #xC0 #x00))
  
  (define-stylesheet $tamale-unit-failed-style 
    :base $tamale-unit-passed-style
    :color (color #xC0 #x00 #x00))
  
  ;;; Display a test report on the current card.
  (define (report-test-results report)
    (define (draw-result style text)
      (draw-text style (rect 100 100 700 175) text))
    (if (test-report-success? report)
      (draw-result $tamale-unit-passed-style "OK")
      (begin
        (draw-result $tamale-unit-failed-style "FAILED")
        (draw-text $tamale-unit-style (rect 100 175 700 500)
                   (apply string-append
                          (map (fn (failure)
                                 (string-append
                                  "^@" (test-failure-title failure) "@^\n"
                                  (test-failure-message failure) "\n\n"))
                               (test-report-failures report)))))))
  
  ;;; Display the results of a set of tests on a card.
  (define-card-template %test-suite%
      [tests]
      ()
    (clear-screen (color #xFF #xFF #xFF))
    (let [[report (make-test-report)]]
      (foreach [test-class tests]
         (run-tests test-class report))
      (report-test-results report)))
  
  ;;========================================================================

  (require-for-syntax (lib "capture.ss" "5L"))
  (provide <test-failure> test-failure-test-case test-failure-exception
           test-failure-title test-failure-message
           <test-report> test-report-success? test-report-success-count
           test-report-failures test-report-failure-count
           test-report-run-count
           <test-method> test-method-title test-method-function
           <test-case-metaclass> test-case-test-methods
           <test-case> test-case-test-method
           setup-test teardown-test add-test-method!
           run-tests run-test-method
           define-test-case-helper define-test-case with-captured-variable
           assert-equal assert-macro-expansion 
           make-test-report 
           fixture-dir)
  
  ;;; Data about a single failed test case.
  (defclass <test-failure> ()
    test-case exception)
  
  ;;; The title of the failed test case.
  (define (test-failure-title failure)
    (test-method-title 
     (test-case-test-method (test-failure-test-case failure))))
  
  ;;; The message associated with the failed test case.
  (define (test-failure-message failure)
    (exn-message (test-failure-exception failure)))
  
  ;;; Data collected from an entire test run.
  (defclass <test-report> ()
    [success? :initvalue #t]
    [success-count :initvalue 0]
    [failures :initvalue '()])
  
  ;;; The number of failures in a test run.
  (define (test-report-failure-count report)
    (length (test-report-failures report)))
  
  ;;; The number of tests executed in a test run.
  (define (test-report-run-count report)
    (+ (test-report-success-count report) (test-report-failure-count report)))
  
  ;;; Add a failed test case to the report.
  (define (report-failure! report test-case exception)
    (set! (test-report-success? report) #f)
    (set! (test-report-failures report) 
          (cons (make-test-failure test-case exception)
                (test-report-failures report))))
  
  ;;; Add a successful test case to the report.
  (define (report-success! report)
    (set! (test-report-success-count report) 
          (1+ (test-report-success-count report))))
  
  ;;; A single test method associated with a test case.
  (defclass <test-method> ()
    title function)
  
  ;;; <test-case> objects are annotated with a list of associated
  ;;; test methods.
  (defclass <test-case-metaclass> (<class>)
    (direct-test-methods :initvalue '() 
                         :accessor test-case-direct-test-methods))
  
  ;;; Find all test methods associated with this class and all 
  ;;; superclasses.  Does not support multiple inheritance.
  (define (test-case-test-methods test-case-class)
    (define direct (test-case-direct-test-methods test-case-class))
    (if (eq? test-case-class <test-case>)
      direct
      (let [[supers (class-direct-supers test-case-class)]]
        (assert-equal 1 (length supers))
        (append direct (test-case-test-methods (first supers))))))
  
  ;;; A test case consists of a setup function, zero or more test
  ;;; methods, and a teardown function.
  (defclass <test-case> ()
    test-method
    :metaclass <test-case-metaclass>)
  
  ;;; Prepare an instance of <test-case> to run a test method.
  ;;; Called once before each test method in the test case.
  (defgeneric (setup-test (test <test-case>)))
  (defmethod (setup-test (test <test-case>))
    #f)

  ;;; Clean up an instance of <test-case> after running a test method.
  ;;; Called once after each test method in the test case.
  (defgeneric (teardown-test (test <test-case>)))
  (defmethod (teardown-test (test <test-case>))
    #f)
  
  ;;; Add a test method to a <test-case> class.
  (define (add-test-method! test-case-class title function)
    (set! (test-case-direct-test-methods test-case-class)
          (cons (make <test-method> 
                      :title title 
                      :function function)
                (test-case-direct-test-methods test-case-class))))
  
  ;;; Run each test method associated with a given <test-case> class.
  (define (run-tests test-case-class report)
    (foreach [meth (test-case-test-methods test-case-class)]
      (run-test-method (make test-case-class :test-method meth) report)))
  
  ;;; Run a single test case method, handling setup, teardown and reporting.
  (define (run-test-method test-case-instance report)
    (define test-method (test-case-test-method test-case-instance))
    (with-handlers [[not-break-exn?
                     (fn (exn) 
                       (report-failure! report test-case-instance exn))]]
      (dynamic-wind
       (fn () (setup-test test-case-instance))
       (fn ()
         ((test-method-function test-method) test-case-instance)
         (report-success! report))
       (fn () (teardown-test test-case-instance)))))
  
  ;;; Define a subclass of <test-case> with setup and teardown functions,
  ;;; and zero or more test cases.
  (define-syntax define-test-case
    (syntax-rules ()
      [(_ class () body ...)
       (define-test-case class (<test-case>) body ...)]
      [(_ class (super) [[var val] ...] body-form ...)
       (begin 
         (defclass class (super)
           (var :initvalue val :accessor var)
           ...
           :metaclass <test-case-metaclass>)
         (define-test-case-helper class body-form) 
         ...)]))
  (define-syntax-indent define-test-case 3)
  
  ;;; Execute BODY, capturing VAR and binding it to the result of
  ;;; EXPR.  Used to implement SELF in test cases.
  (define-syntax (with-captured-variable stx)
    (syntax-case stx ()
      [(_ [var expr] . body)
       (let* [[new-var-name (syntax-object->datum #'var)]
              [new-var (make-capture-var/ellipsis #'body new-var-name)]]
         (quasisyntax/loc stx
           (let ((#,new-var expr)) . body)))]))
  (define-syntax-indent with-captured-variable 1)
  
  ;;; Expand a SETUP, TEARDOWN, or TEST form.
  (define-syntax define-test-case-helper
    (syntax-rules (setup teardown test)
      [(_ class (setup body ...))
       (defmethod (setup-test (self class))
         (call-next-method)
         (with-captured-variable [self self] body ...))]
      [(_ class (teardown body ...))
       (defmethod (teardown-test (self class))
         (with-captured-variable [self self] body ...)
         (call-next-method))]
      [(_ class (test title body ...))
       (add-test-method! class title 
                         (fn (self)
                           (with-captured-variable [self self] body ...)))]))
  (define-syntax-indent define-test-case-helper 1)
  (define-syntax-indent setup 0)
  (define-syntax-indent teardown 0)
  (define-syntax-indent test 1)
  
  ;;; Assert that an expression returns the expected value.
  (define-syntax assert-equal 
    (syntax-rules ()
      [(_ expected value)
       (let [[e expected] [v value]]
         (unless (equal? e v)
           (error (cat "Expected <" e ">, got <" v "> in <" 'value ">"))))]))
  (define-syntax-indent assert-equal function)
  
  ;;; Assert that the specified macro, expanded once, returns the
  ;;; expected source code.  This is most useful for macros which
  ;;; have a well-defined mapping to a public API.
  (define-syntax assert-macro-expansion
    (syntax-rules ()
      [(_ expansion source)
       (assert-equal
        'expansion
        (syntax-object->datum (expand-once #'source)))]))
  (define-syntax-indent assert-macro-expansion function)
  
  (define (fixture-dir name)
    (build-path (current-directory) "Runtime" "5L" (cat name "-fixtures")))
  )