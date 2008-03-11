;; PORTED
;; Some unit tests, mostly written by Robinson.  These use the
;; semi-supported TEST-ELEMENTS form. 
(module test-cases (lib "halyard.ss" "halyard")
  (require (lib "tamale-unit.ss" "halyard"))
  (require (lib "errortrace-lib.ss" "errortrace"))
  
  (sequence test-cases)

  ;;=======================================================================
  ;;  Helper Functions (to be factored-out later)
  ;;=======================================================================
  
  (define $chars-that-must-be-quoted-for-regexp
    (string->list "()*+?[]{}.^\\|"))

  ;;; Given a STRING str, quote any chars that are considered "special" by
  ;;; scheme regular expressions so that we can use 'str' as a simple string
  ;;; match.
  (define (quote-for-regexp str)
    (define (quote-single-for-regexp c)
      (define s (string c))
      ;; Chars we must quote.
      (if (member? c $chars-that-must-be-quoted-for-regexp)
        (cat "\\" s)
        s))
    (define result 
      (apply cat (map quote-single-for-regexp (string->list str))))
    ;;(5l-log (cat "quote-for-regexp: " result))
    result)
  
  ;;=======================================================================
  ;;  Element Syntax test cases
  ;;=======================================================================
  
  (define-test-case <element-test> () []
    (test-elements "Creating an element with an invalid parameter should fail"
      (define-class %invalid-parameter-template% (%invisible-element%)
        (assert-raises exn:fail? (%invalid-parameter-template% .new
                                   :name 'foo :zarqan 1))))
    )
  
  (card test-cases/element-test
      (%test-suite%
       :tests (list <element-test>)))
  
  
  ;;=======================================================================
  ;;  Custom Element test cases
  ;;=======================================================================

  (define (test-elements-full-name node-name)
    (cat ((current-card) .full-name) "/temporary-parent/" node-name))
  
  (define-test-case <custom-element-test> () []
    (test-elements "Creating a %custom-element%"
      (%custom-element% .new :bounds (rect 0 0 10 10)))
    (test-elements "Setting the shape of a %custom-element%"
      (define new-shape (rect 0 0 5 5))
      (define foo (%custom-element% .new :bounds (rect 0 0 10 10)))
      (set! (foo .shape) new-shape)
      ;; The shape should be correctly updated.
      (assert-equals new-shape (foo .shape)))
    (test-elements 
        "Creating a %custom-element% with a negative shape should error-out"
      (define original-shape (rect 10 10 0 0))
      (define elem-name 'foo-negative-shape-error)
      (define expected-error
        (quote-for-regexp (cat "has negative size")))
      
      (assert-raises-message exn:fail? expected-error
        (%custom-element% .new :name elem-name :bounds original-shape)))
    (test-elements "SET!ing a %custom-element% to a negative-shape should fail"
      (define original-shape (rect 0 0 10 10))
      (define foo (%custom-element% .new :bounds original-shape))
      
      ;; This should raise a 'veto' exception (but I don't think we have 
      ;; specified such an exception type).
      (assert-raises exn:fail?
        (set! (foo .shape) (rect 10 10 0 0)))
      
      ;; Because of the veto, the shape should remain unchanged.
      (assert-equals original-shape (foo .shape))))
  
  (card test-cases/custom-element-test
      (%test-suite%
       :tests (list <custom-element-test>)))
  
  ;;=======================================================================
  ;;  Node test cases
  ;;=======================================================================
  
  (define-class %foo% ())
  
  (define (node-full-name-error item)
    (quote-for-regexp "full-name"))
  
  (define-test-case <node-full-name-test> () []
    (test "node-full-name should succeed on a running node or node-path"
          (define hyacinth (new-box (rect 0 0 10 10) :name 'rose))
          (hyacinth .full-name)
          (@rose .full-name))
    (test "node-full-name should succeed on a static node"
          (define hyacinth (card-next))
          (hyacinth .full-name))
    (test "node-full-name should fail on a static node-path"
          (define hyacinth @next-test-card)
          (define rose @test-cases/foo/bar/baz/wonky)
          (define (nfn-static-error item)
            (cat "Cannot find " item "; "
                 "If referring to a static node, please resolve it first."))
          (assert-raises-message exn:fail? (nfn-static-error hyacinth)
            (hyacinth .full-name))
          (assert-raises-message exn:fail? (nfn-static-error rose)
            (rose .full-name)))
    (test "node-full-name should fail on a non-node ruby object"
          (define hyacinth (%foo% .new))
          (assert-raises-message exn:fail? (node-full-name-error hyacinth)
            (hyacinth .full-name)))
    (test "node-full-name should fail on a swindle object"
          (define hyacinth (make-vector 1))
          (assert-raises-message exn:fail? (node-full-name-error hyacinth)
            (hyacinth .full-name)))
    (test "node-full-name should fail on a string or integer"
          (define hyacinth "pretty flowers")
          (define rose 34)
          
          (assert-raises-message exn:fail? (node-full-name-error hyacinth) 
            (hyacinth .full-name))
          (assert-raises-message exn:fail? (node-full-name-error rose)
            (rose .full-name))))
  
  (card test-cases/node-test
      (%test-suite%
       :tests (list <node-full-name-test>)))
  
  ;; We need to have a next-card for one of our node-full-name tests.
  (card test-cases/next-test-card
      (%test-suite% :tests '()))
  
  
  ;;=======================================================================
  ;;  Browser test cases
  ;;=======================================================================
  
  (define-class %test-browser% (%browser%)
    (default rect (rect 0 0 100 100)))
  
  (define (browser-native-path path)
    (make-native-path "HTML" path))
  
  (define-test-case <browser-simple-test> () []
    (test-elements "The browser should load with default values"
      (%test-browser% .new))
    (test-elements "The browser should load a local HTML page"
      (%test-browser% .new :path "sample.html"))
    (test-elements "The browser should load 'about:blank'"
      (%test-browser% .new :path "about:blank"))
    (test-elements 
        "The browser should fail to load a non-existent local HTML page"
      (define non-existent-file "foo-bar-not-here.html")
          (assert-raises-message exn:fail? 
            (quote-for-regexp
             (cat "No such file: " (browser-native-path non-existent-file)))
            (%test-browser% .new :path non-existent-file)))
    (test-elements "The browser should load a local HTML page using file:///"
          (%test-browser% .new :path "file:///sample.html"))
    (test-elements "The browser should load an external HTML page via http"
          (%test-browser% .new :path "http://www.google.com"))
    (test-elements "The browser should load an ftp site"
          (%test-browser% .new :path "ftp://ftp.dartmouth.edu/"))
    ;; This page doesn't throw a Tamale error, but IE doesn't seem to be able
    ;; to work with the gopher protocol.
    (test-elements "The browser should load a gopher site"
          (%test-browser% .new :path "gopher://home.jumpjet.info/"))
    (test-elements 
     "The browser should load URLs with ampersands (&amp;) in them"
          (%test-browser% .new :path "http://www.google.com/&foo=bar"))
    (test-elements "The browser should accept a zero-sized rect"
          (%test-browser% .new :rect (rect 0 0 0 0))))
  
  (card test-cases/native-browser-tests
      (%test-suite%
       :tests (list <browser-simple-test>)))
  
  (define-test-case <fallback-browser> () []
    ;;; NOTE: the default path of "about:blank" appears to hang the
    ;;; fallback browser.
    (test-elements "The fallback browser should load local files"
      (%test-browser% .new :fallback? #t :path "sample.html")))
  
  ;;; NOTE: 
  (card test-cases/integrated-browser-tests
      (%test-suite%
       :tests (list <fallback-browser>)))
  
  ;;=======================================================================
  ;;  Graphic test cases
  ;;=======================================================================
  
  (define-test-case <graphic-element-test> () []
    (test-elements "Creating a non-alpha %graphic%"
      (%graphic% .new :at (point 0 0) :path "but40.png"))
    (test-elements "Creating an alpha %graphic%"
      (%graphic% .new :at (point 0 0) :alpha? #t :path "lbul.png"))
    (test-elements "Setting the path should change graphic and shape"
      (define orig-graphic "but40.png")
      (define new-graphic "but70.png")
      (define new-graphic-shape (measure-graphic new-graphic))
      (define foo (%graphic% .new :at (point 0 0) :path orig-graphic))
      (set! (foo .path) new-graphic)
      
      ;; The path should be correctly updated.
      (assert-equals new-graphic (foo .path))
      ;; The shape should be correctly updated.
      (assert-equals new-graphic-shape (foo .shape))))
  
  (card test-cases/graphic-test
      (%test-suite%
       :tests (list <graphic-element-test>)))
  
  ;;=======================================================================
  ;;  Errortrace test cases
  ;;=======================================================================
  
  (define a #f)
  (define method-error-test #f)
  
  (parameterize [[current-compile errortrace-compile-handler]
                 [use-compiled-file-paths (list (build-path "compiled" 
                                                       "errortrace")
                                           (build-path "compiled"))]]
    (set! a (dynamic-require '(file "errortrace-test.ss") 'a))
    (set! method-error-test (dynamic-require '(file "errortrace-test.ss") 
                                             'method-error-test)))

  (define-syntax return-errortrace
    (syntax-rules ()
      [(_ expr) 
       (with-handlers [[exn:fail? (fn (exn) 
                                    (define port (open-output-string))
                                    (print-error-trace port exn)
                                    (get-output-string port))]]
         expr)]))

  (define-syntax assert-matches
    (syntax-rules ()
      [(_ regexp expr)
       (let [[val expr]]
         (unless (regexp-match regexp val)
           (error (cat "Expected " 'expr " to match " regexp
                       ", got " val " instead."))))]))
  
  (define-test-case <errortrace-test> () []
    (test "Errotrace should include all stack frames"
      (define trace (return-errortrace (a '())))
      (assert-matches "\\(\\+ 1 n\\)" trace)
      (assert-matches "\\(\\+ 1 \\(i n\\)\\)" trace)
      (assert-matches "\\(\\+ 1 \\(h n\\)\\)" trace)
      (assert-matches "\\(\\+ 1 \\(b n\\)\\)" trace))
    (test "Errortrace should include correct line numbers"
      (define trace (return-errortrace (a '())))
      (assert-matches "errortrace-test\\.ss:13:4" trace)
      (assert-matches "errortrace-test\\.ss:16:4" trace)
      (assert-matches "errortrace-test\\.ss:37:4" trace))
    (test "Errortrace should work for errors in methods"
      (define trace (return-errortrace (method-error-test)))
      (assert-matches "\\(send% t 'test-method \"hello!\"\\)" trace)
      (assert-matches "\\(\\* \\(send% b 'test-me x\\) \\(send% self 'foo\\)\\)"
                      trace)
      (assert-matches "\\(\\+ \\(send% self 'bar\\) x\\)" trace)))

  (card test-cases/errortrace-test
      (%test-suite%
       :tests (list <errortrace-test>)))
  )
