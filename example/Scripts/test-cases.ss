(module test-cases (lib "5l.ss" "5L")
  (require (lib "tamale-unit.ss" "5L"))
  
  (sequence test-cases)

  ;;=======================================================================
  ;;  Helper Functions (to be factored-out later)
  ;;=======================================================================
  
  ;;; Given a STRING str, quote any chars that are considered "special" by
  ;;; scheme regular expressions so that we can use 'str' as a simple string
  ;;; match.
  (define $chars-that-must-be-quoted-for-regexp
    (string->list "()*+?[]{}.^\\|"))
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
  
  (define-test-case <element-syntax-test> () []
    (test-elements "Defining an element template should succeed."
      (define-element-template %empty-template%
          []
          ()))
    (test-elements "Defining an element template with variables and a parent template should succeed"
      (define-element-template %template-with-stuff%
          [one two three]
          (%element%)))
    (test-elements "Defining an element template with improper syntax should give a syntax error."
      ;; XXXX - The Tamale-Unit framework needs to be extended to allow us to
      ;; add "known bug" unit tests into the system.
      ;; This DEFINE should raise a syntax error of some sort -- something like
      ;; "Improper syntax for define-element-template on line 234:
      ;;  (define-element-template %invalid-syntax-template%"
      (define-element-template %invalid-syntax-template%
          []
          []
        (%element%)))
    (test-elements "Defining an element template with improper syntax should give a syntax error (2)."
      (define maple-syrup "ding")
      ;; XXX - this currently raises an 'unbound variable' error, but
      ;; it really should be raising a syntax error.
      ;; Actually, if maple-syrup is not defined, it gives an unbound variable
      ;; error, but once it is defined it will give a struct:exn:fail:contract
      ;; with 'procedure application: expected procedure, given: "ding"...'
      (assert-raises-message exn:fail? 
        ;;"unbound variable in module in: maple-syrup"
        "procedure application: expected procedure, given: \"ding\""
        ;; Testing
        (define-element-template %invalid-syntax-template-2% []
            [[maple-syrup :type <string> :default "foo"]]
          (%element%))))
    (test-elements "A element template will fail if it inherits from something other than another template."
      (define foo 4)
      (assert-raises-message exn:fail? "Not a Ruby-style object"
        (define-element-template %not-derived-from-ruby-object-template%
            []
            (foo)))))
  
  (define-test-case <element-test> () []
    (test-elements "Creating an element with an invalid parameter should fail"
      (define-element-template %invalid-parameter-template%
          []
          ())
      (assert-raises exn:fail? (create %invalid-parameter-template% 
                                       :name 'foo :zarqan 1))))
  
  (card test-cases/element-test
      (%test-suite%
       :tests (list <element-syntax-test>
                    <element-test>)))
  
  ;;=======================================================================
  ;;  Custom Element test cases
  ;;=======================================================================

  (define (test-elements-full-name node-name)
    (cat (node-full-name (current-card)) "/temporary-parent/" node-name))
  
  (define-test-case <custom-element-test> () []
    (test-elements "Creating a %custom-element%"
      (create %custom-element%
              :shape (rect 0 0 10 10)))
    (test-elements "Setting the shape of a %custom-element%"
      (define new-shape (rect 0 0 5 5))
      (define foo (create %custom-element%
                          :shape (rect 0 0 10 10)))
      (set! (foo .shape) new-shape)
      
      ;; The shape should be correctly updated.
      (assert-equals new-shape (foo .shape)))
    (test-elements 
        "Creating a %custom-element% with a negative shape should error-out"
      (define original-shape (rect 10 10 0 0))
      (define elem-name 'foo-negative-shape-error)
      (define expected-error
        (quote-for-regexp
         (cat "%custom-element%: " (test-elements-full-name elem-name)
              " may not have a negative-sized shape: "
              original-shape ".")))
      
      (assert-raises-message exn:fail? expected-error
        (create %custom-element%
                :name elem-name
                :shape original-shape))
      ;; The element will be created, but should have zero size.
      (assert-equals (rect 0 0 0 0) 
                     (@temporary-parent/foo-negative-shape-error .shape)))
    (test-elements "SET!ing a %custom-element% to a negative-shape should fail"
      (define original-shape (rect 0 0 10 10))
      (define foo (create %custom-element%
                          :shape original-shape))
      
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
    (quote-for-regexp 
     (cat "node-full-name: expecting node or node-path, given " 
          item ".")))
  
  (define-test-case <node-full-name-test> () []
    (test "node-full-name should succeed on a running node or node-path"
          (define hyacinth (box (rect 0 0 10 10) :name 'rose))
          (node-full-name hyacinth)
          (node-full-name @rose))
    (test "node-full-name should succeed on a static node"
          (define hyacinth (card-next))
          (node-full-name hyacinth))
    (test "node-full-name should fail on a static node-path"
          (define hyacinth @next-test-card)
          (define rose @test-cases/foo/bar/baz/wonky)
          (define (nfn-static-error item)
            (cat "Cannot find " item "; "
                 "If referring to a static node, please resolve it first."))
          (assert-raises-message exn:fail? (nfn-static-error hyacinth)
            (node-full-name hyacinth))
          (assert-raises-message exn:fail? (nfn-static-error rose)
            (node-full-name rose)))
    (test "node-full-name should fail on a non-node ruby object"
          (define hyacinth (%foo% .new))
          (assert-raises-message exn:fail? (node-full-name-error hyacinth)
            (node-full-name hyacinth)))
    (test "node-full-name should fail on a swindle object"
          (define hyacinth (make-vector 1))
          (assert-raises-message exn:fail? (node-full-name-error hyacinth)
            (node-full-name hyacinth)))
    (test "node-full-name should fail on a string or integer"
          (define hyacinth "pretty flowers")
          (define rose 34)
          
          (assert-raises-message exn:fail? (node-full-name-error hyacinth) 
            (node-full-name hyacinth))
          (assert-raises-message exn:fail? (node-full-name-error rose)
            (node-full-name rose))))
  
  (card test-cases/node-test
      (%test-suite%
       :tests (list <node-full-name-test>)))
  
  ;; We need to have a next-card for one of our node-full-name tests.
  (card test-cases/next-test-card
      (%test-suite% :tests '()))
  
  ;;=======================================================================
  ;;  Browser test cases
  ;;=======================================================================
  
  (define-element-template %test-browser%
      [[rect :new-default (rect 0 0 100 100)]]
      (%browser%))
  
  (define (browser-native-path path)
    (make-native-path "HTML" path))
  
  (define-test-case <browser-simple-test> () []
    (test-elements "The browser should load with default values"
      (create %test-browser%))
    (test-elements "The browser should load a local HTML page"
      (create %test-browser%
              :path "sample.html"))
    (test-elements "The browser should load 'about:blank'"
      (create %test-browser%
              :path "about:blank"))
    (test-elements 
        "The browser should fail to load a non-existent local HTML page"
      (define non-existent-file "foo-bar-not-here.html")
          (assert-raises-message exn:fail? 
            (quote-for-regexp
             (cat "No such file: " (browser-native-path non-existent-file)))
            (create %test-browser%
                    :path non-existent-file)))
    (test-elements "The browser should load a local HTML page using file:///"
          (create %test-browser%
                  :path "file:///sample.html"))
    (test-elements "The browser should load an external HTML page via http"
          (create %test-browser%
                  :path "http://www.google.com"))
    (test-elements "The browser should load an ftp site"
          (create %test-browser%
                  :path "ftp://ftp.dartmouth.edu/"))
    ;; This page doesn't throw a Tamale error, but IE doesn't seem to be able
    ;; to work with the gopher protocol.
    (test-elements "The browser should load a gopher site"
          (create %test-browser%
                  :path "gopher://home.jumpjet.info/"))
    (test-elements 
     "The browser should load URLs with ampersands (&amp;) in them"
          (create %test-browser%
                  :path "http://www.google.com/&foo=bar"))
    (test-elements "The browser should accept a zero-sized rect"
          (create %test-browser%
                  :rect (rect 0 0 0 0))))
  
  (card test-cases/native-browser-tests
      (%test-suite%
       :tests (list <browser-simple-test>)))
  
  (define-test-case <fallback-browser> () []
    ;;; NOTE: the default path of "about:blank" appears to hang the
    ;;; fallback browser.
    (test-elements "The fallback browser should load local files"
      (create %test-browser%
              :fallback? #t
              :path "sample.html")))
  
  ;;; NOTE: 
  (card test-cases/integrated-browser-tests
      (%test-suite%
       :tests (list <fallback-browser>)))
  
  ;;=======================================================================
  ;;  Graphic test cases
  ;;=======================================================================
  
  (define-test-case <graphic-element-test> () []
    (test-elements "Creating a non-alpha %graphic%"
      (create %graphic%
              :path "but40.png"))
    (test-elements "Creating an alpha %graphic%"
      (create %graphic%
              :alpha? #t
              :path "lbul.png"))
    (test-elements "Setting the path should change graphic and shape"
      (define orig-graphic "but40.png")
      (define new-graphic "but70.png")
      (define new-graphic-shape (measure-graphic new-graphic))
      (define foo (create %graphic%
                          :path orig-graphic))
      (set! (foo .path) new-graphic)
      
      ;; The path should be correctly updated.
      (assert-equals new-graphic (foo .path))
      ;; The shape should be correctly updated.
      (assert-equals new-graphic-shape (foo .shape))))
  
  (card test-cases/graphic-test
      (%test-suite%
       :tests (list <graphic-element-test>)))
  
  )
