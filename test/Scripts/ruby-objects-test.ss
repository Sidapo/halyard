;; PORTED
(module ruby-objects-test (lib "5l.ss" "5L")

  (require (lib "tamale-unit.ss" "5L"))

  (sequence ruby-objects-test)
  
  
  ;;=======================================================================
  ;;  Swindle-specific class/object operators
  ;;=======================================================================
  ;; These are the original swindle class/object operators, given new names
  ;; to avoid confusion with the 'universal' object operators.
  
  (define-test-case <swindle-class-operators> () []
    (test "A (rect ...) should be a swindle object"
          (assert (swindle-object? (rect 0 0 10 10)))
          (assert (not (swindle-object? 3))))
    (test "The class <rect> should be a swindle class"
          (assert (swindle-class? <rect>))
          (assert (not (swindle-class? (rect 0 0 10 10)))))
    (test "A (rect ...) should be an instance of <rect>"
          (assert (swindle-instance-of? (rect 0 0 10 10) <rect>))
          (assert (not (swindle-instance-of? (rect 0 0 10 10) <point>))))
    (test "<rect> should be a subclass of <shape>"
          (assert (swindle-subclass? <rect> <shape>))
          (assert (not (swindle-subclass? <polygon> <rect>)))
          (assert (not (swindle-subclass? <point> <rect>))))          
    )
  
  
  ;;=======================================================================
  ;;  Object Model
  ;;=======================================================================

  (define-class %foo% ()
    ;; Create a method the hard way.
    (.define-method 'hey (method () "Hey!"))

    ;; Create a member variable the hard way.
    (def (name)
      (slot 'name))
    (def (set-name! val)
      (set! (slot 'name) val))

    ;; Now, try it with a wrapper function.
    (.attr 'street :default (method () "J. Random Ave") :writable? #t)

    ;; Finally, let's use our macro.
    (attr city "Somewhere" :writable? #t)

    ;; An attribute with a default value.
    (attr state "Freedonia")
    )
  
  (define-class %frob% (%foo%)
    (attr title "Peon")
    (attr planet))

  (define-class %bar% ()
    (def (hello) "Hello!")
    (def (hello-to name) (cat (.hello) " " name "!")))
  (define-class %baz% (%bar%)
    (def (hello) (cat (super) " From Baz!")))

  (define-class %friendly% ()
    (def (method-missing name . args)
      "Hi!"))  
  
  
  ;;=======================================================================
  ;;  Universal class/object operators
  ;;=======================================================================
  ;; These are the new operators (that override the swindle names for class
  ;; operators. They should work on both swindle and ruby objects.
  
  (define-test-case <universal-class-operators> () []
    (test "Ruby and Swindle instances should both be objects "
          (define foo (%foo% .new))
          (define r (rect 0 0 10 10))
          (assert (object? foo))
          ;; For Ruby Objects, classes are objects, too.
          (assert (object? %foo%))
          (assert (object? r))
          (assert (not (object? 3)))
          (assert (not (object? '(1 2 3)))))
    (test "Ruby and Swindle classes should both pass 'class?'"
          (define foo (%foo% .new))
          (define r (rect 0 0 10 10))
          (assert (class? %foo%))
          (assert (class? %frob%))
          (assert (class? <rect>))
          (assert (class? <point>))
          (assert (not (class? 3)))
          (assert (not (class? r))))
    (test "Ruby and Swindle instances should both pass 'instance-of?' "
          (define foo (%foo% .new))
          (define bar (%bar% .new))
          (define frob (%frob% .new :planet 'saturn))
          (define r (rect 0 0 10 10))
          (assert (instance-of? foo %foo%))
          (assert (instance-of? bar %bar%))
          (assert (not (instance-of? foo %bar%)))
          ;; %frob% is a subclass of %foo%, so this should be true.
          (assert (instance-of? frob %foo%))
          (assert (not (instance-of? foo %frob%)))
          (assert (instance-of? r <rect>))
          (assert (instance-of? r <shape>))
          ;; Primitive types should also work...
          (assert (instance-of? 3 <integer>))
          (assert (instance-of? '(1 2 3) <list>))
          (assert (instance-of? "Black Adder" <string>))
          (assert (not (instance-of? r <point>))))
    (test "Ruby and Swindle instances should both pass 'subclass?'"
          (define foo (%foo% .new))
          (define bar (%bar% .new))
          (define frob (%frob% .new :planet 'saturn))
          (define r (rect 0 0 10 10))
          (assert (subclass? %frob% %foo%))
          (assert (not (subclass? %foo% %frob%)))
          (assert (not (subclass? %foo% %bar%)))
          (assert (subclass? <rect> <shape>))
          (assert (subclass? <polygon> <shape>))
          (assert (not (subclass? <rect> <polygon>)))
          (assert (not (subclass? r <rect>))))
    (test "Ruby and Swindle instances: special cases of 'subclass?'"
          (define foo (%foo% .new))
          (assert (not (subclass? <rect> %foo%)))
          (assert (not (subclass? %foo% <rect>))))
    )

  (card ruby-objects-test/classes
      (%test-suite%
       :tests (list <swindle-class-operators>
                    <universal-class-operators>)))
  
  
  ;;=======================================================================
  ;;  Ruby Object Tests
  ;;=======================================================================
  
  (define-test-case <ruby-object-test> () []
    (test "Objects should be instances of their class"
      (define foo (%foo% .new))
      (assert (foo .instance-of? %foo%))
      (assert (not (foo .instance-of? %bar%))))
    (test "An object should know its class"
      (define foo (%foo% .new))
      (assert (eq? (foo .class) %foo%))
      (define bar (%bar% .new))
      (assert (eq? (bar .class) %bar%)))
    (test "Objects should be unique"
      (define foo1 (%foo% .new))
      (define foo2 (%foo% .new))
      (assert (not (eq? foo1 foo2))))
    (test "Objects should be instances of their superclass"
      (define baz (%baz% .new))
      (assert (baz .instance-of? %baz%))
      (assert (baz .instance-of? %bar%))
      (assert (baz .instance-of? %object%)))
    (test "Objects should implement their methods"
      (define bar (%bar% .new))
      (assert-equals "Hello!" (bar .hello))
      (assert-equals "Hello!" (bar .send 'hello '())))
    (test "DEF methods should support implicit self"
      (define bar (%bar% .new))
      (assert-equals "Hello! Mark!" (bar .hello-to "Mark")))
    (test "Calling super should call parent class's method"
      (define baz (%baz% .new))
      (assert-equals "Hello! From Baz!" (baz .hello))
      (assert-equals "Hello! From Baz! Mark!" (baz .hello-to "Mark"))
      (assert-equals "Hello! From Baz! Mark!" (baz .send 'hello-to '("Mark"))))
    (test "SLOT should allow access to private storage slots"
      (define foo (%foo% .new))
      (set! (foo .name) "Foonly")
      (assert-equals "Foonly" (foo .name)))
    (test ".ATTR with :WRITABLE? #t should create a getter and setter"
      (define foo (%foo% .new))
      (set! (foo .street) "Quux Street")
      (assert-equals "Quux Street" (foo .street)))
    (test "ATTR with :WRITABLE? #t should create a getter and setter"
      (define foo (%foo% .new))
      (set! (foo .city) "Zotville")
      (assert-equals "Zotville" (foo .city)))
    (test ".NEW should support keyword arguments"
      (define foo (%foo% .new
                     :name "Fooner" :street "Xyzzy Ave." 
                     :city "Colossal City"))
      (assert-equals "Fooner" (foo .name))
      (assert-equals "Xyzzy Ave." (foo .street))
      (assert-equals "Colossal City" (foo .city))
      (assert-equals "Freedonia" (foo .state)))
    (test ".NEW should initialize superclasses correctly"
      (define frob (%frob% .new 
                     :name "Froobly" :street "Infinite Loop"
                     :city "Reality Distortion" :planet "Zarqhuan"
                     :title "Grand Poobah"))
      (assert-equals "Froobly" (frob .name))
      (assert-equals "Infinite Loop" (frob .street))
      (assert-equals "Reality Distortion" (frob .city))
      (assert-equals "Freedonia" (frob .state))
      (assert-equals "Zarqhuan" (frob .planet))
      (assert-equals "Grand Poobah" (frob .title)))
    (test "METHOD-MISSING should be called when a method doesn't exist"
      (define friendly (%friendly% .new))
      (assert-equals "Hi!" (friendly .hello))
      (assert-equals "Hi!" (friendly .hey-there)))
    (test "Objects should have a reasonable print representation"
      (assert-equals "#<%foo%>" (cat (%foo% .new)))
      (assert-equals "%foo%" (cat %foo%)))
    )
  
  (define-class %quux-1% ()
    (attr a 1 :writable? #t)
    (attr b 10)
    (attr int-only 0 :type <integer>))

  (define-class %quux-1.1% (%quux-1%)
    (value a 2)
    (default b 20))

  (define-class %quux-1.1.1% (%quux-1.1%)
    (attr c 100)
    (attr d :mandatory? #f)
    (value b (+ (.c) (.d))  :skip-if-missing-values? #t))
  
  (define-class %quux-1.1.2% (%quux-1.1%)
    (attr d :mandatory? #f)
    (value b (.d)))

  (define-class %quux-2% ()
    (attr a)) ; Must be specified.

  (define-test-case <ruby-new-test> () []
    (test "Creating a class with .new should fail"
      (assert-raises exn:fail? (%class% .new)))
    (test "ATTR should support default values"
      (define q (%quux-1% .new))
      (assert-equals 1 (q .a))
      (assert-equals 10 (q .b)))
    (test "VALUE should override the default value"
      (define q (%quux-1.1% .new))
      (assert-equals 2 (q .a)))
    (test "VALUE should be inherited by subclasses"
      (define q (%quux-1.1.1% .new))
      (assert-equals 2 (q .a)))
    (test "VALUE should prevent the ATTR from being initialized with .NEW"
      (assert-raises exn:fail? (%quux-1.1% .new :a 3)))
    (test "DEFAULT should override the original default value"
      (define q (%quux-1.1% .new))
      (assert-equals 20 (q .b)))
    (test "DEFAULT should be inherited by subclasses"
      (define q (%quux-1.1.1% .new))
      (assert-equals 20 (q .b)))
    (test "DEFAULT should do nothing if the attribute was passed in"
      (define q (%quux-1.1% .new :b 30))
      (assert-equals 30 (q .b)))
    (test "VALUE should be computable from other values"
      (define q (%quux-1.1.1% .new :d 1000))
      (assert-equals 1100 (q .b)))
    (test "VALUE should fail if it computes from a missing value"
      (assert-raises exn:fail? (%quux-1.1.2% .new)))
    (test "VALUE should optionally be able to skip missing values"
      (define q (%quux-1.1.1% .new))
      (assert-equals 20 (q .b)))
    (test "ATTR should support :WRITABLE?"
      (define q (%quux-1% .new))
      (set! (q .a) 2)
      (assert-equals 2 (q .a))
      (assert-raises exn:fail? (set! (q .b) 20)))
    (test "Failure to initialize an attribute should raise an error"
      (assert-raises exn:fail? (%quux-2% .new)))
    (test "Creating an object with an incorrectly-typed attribute should fail"
      (assert-raises exn:fail? (%quux-1% .new :int-only "foo")))
    (test "Assigning a value of an incorrect type to an attribute should fail"
      (define q (%quux-1% .new))
      (assert-raises exn:fail? (set! (q .int-only) "foo")))
    (test "Initializing a non-existant attribute should fail"
      (assert-raises exn:fail? (%quux-1.1.1% .new
                                 :c 300 :d 12 :nosuch 2)))
    )

  (define-class %responder% ()
    (attr other-method 'trickier)

    (def (method-missing name . args)
      (cond
       [(eq? name 'tricky)
        "Hello"]
       [(eq? name (.other-method))
        "Hi!"]
       [else
        (super)]))

    (with-instance (.class)
      ;; Hyper-formal correctness: By overriding instances-respond-to?, we
      ;; do a bit more work, but we don't need to instatiate the class to
      ;; see what methods it supports.  In ordinary cases, I'm not sure
      ;; that there's any reason to do this instead of using responds-to?,
      ;; as seen below.
      (def (instances-respond-to? name)
        (if (eq? name 'tricky)
            #t
            (super))))

    ;; Since the name of this method is defined at runtime by an instance
    ;; variable, it's impossible to check it at the class level.
    (def (responds-to? name)
      (if (eq? name (.other-method))
        #t
        (super)))
    )

  (define-class %responder-2% (%responder%))

  (define-test-case <ruby-responds-to-test> () []
    (test "Classes should support instances-respond-to?"
      (assert (%foo% .instances-respond-to? 'hey))
      (assert (not (%foo% .instances-respond-to? 'nosuch))))

    (test "Instances should support responds-to?"
      (define foo (%foo% .new))
      (assert (foo .responds-to? 'hey))
      (assert (not (foo .responds-to? 'nosuch))))

    (test "responds-to? and instances-respond-to? should inherit correctly"
      (define responder (%responder-2% .new))
      (assert (responder .responds-to? 'tricky))
      (assert (responder .responds-to? 'trickier)))

    (test "instances-respond-to? is based on classes, not instance behavior"
      (assert (%responder-2% .instances-respond-to? 'tricky))
      (assert (not (%responder-2% .instances-respond-to? 'trickier?))))
    )

  (define-class %fancy-1% ()
    (.define-class-method 'fancy-attr
                          (method (name)
                            (.attr name :default (method () "Fancy!"))))
    (.fancy-attr 'fancy1)

    (with-instance (.class)
      (attr class-attr 'default)))

  (define-class %fancy-2% (%fancy-1%)
    (.define-class-method 'fancy-2-method
                          (method () "Fancy 2"))
    (.fancy-attr 'fancy2))

  ;; The metaclass situation is pretty tricky, and will make much more
  ;; sense if you take a look at the accompanying diagrams, which should be
  ;; in ruby-objects.png in this directory (or ruby-objects.zip for the XMI
  ;; source).  The metaclass system is largely based on the Ruby and
  ;; SmallTalk systems, as documented here:
  ;;
  ;; <http://case.lazaridis.com/attachment/wiki/RubyObjectModel/
  ;;   TheRubyObjectModel.png>
  ;; <http://www.ifi.unizh.ch/richter/Classes/oose2/05_Metaclasses/
  ;;   02_smalltalk/02_metaclasses_smalltalk.html>
  (define-test-case <ruby-metaclass-test> () []
    (test "Classes should support class methods"
      ;; (Which are technically instance methods on their metaclass.)
      (define fancy (%fancy-1% .new))
      (assert-equals "Fancy!" (fancy .fancy1)))
    (test "Classes should not all be direct instances of the same class"
      ;; (Because they should be instances of their metaclass.)
      (assert (not (eq? (%fancy-1% .class) (%fancy-2% .class)))))
    (test "%object% and %class% should not be direct instances of %class%"
      ;; (Because they should be instances of their metaclass.)
      (assert (not (eq? (%object% .class) %class%)))
      (assert (not (eq? (%class% .class) %class%))))
    (test "Class methods should be visible to subclasses"
      ;; metaclass:%fancy-2% should inherit from metaclass:%fancy-1%, which
      ;; should inherit from metaclass:%object%, which should inherit from
      ;; %class%.
      (define fancy (%fancy-2% .new))
      (assert-equals "Fancy!" (fancy .fancy1))
      (assert-equals "Fancy!" (fancy .fancy2)))
    (test "Class methods should not be visible to superclasses"
      (assert-equals "Fancy 2" (%fancy-2% .fancy-2-method))
      (assert-raises exn:fail? (%fancy-1% .fancy-2-method)))
    (test "Class methods should execute on the right class"
      ;; This was a dumb error in my first design.  Let's make sure it
      ;; doesn't come back.
      (define fancy (%fancy-1% .new))
      (assert-equals "Fancy!" (fancy .fancy1))
      (assert-raises exn:fail? (fancy .fancy2)))
    (test "Class attributes should default correctly"
      (assert-equals (%fancy-1% .class-attr) 'default)
      (assert-equals (%fancy-2% .class-attr) 'default))
    )

  (card ruby-objects-test/objects
      (%test-suite%
       :tests (list <ruby-object-test>
                    <ruby-new-test>
                    <ruby-responds-to-test>
                    <ruby-metaclass-test>)))


  ;;=======================================================================
  ;;  ADVISE Tests
  ;;=======================================================================

  (define-class %advised% ()
    (attr events '() :writable? #t)
    (def (record-event! name)
      (set! (.events) (append (.events) (list name))))

    (def (foo)
      (.record-event! 'foo)
      'foo-result)
    (advise before (foo)
      (.record-event! 'before-foo))
    (advise after (foo)
      (.record-event! 'after-foo))
    
    (def (call-super-from-advice)
      (void))
    (advise after (call-super-from-advice)
      (super))
    
    ;; If we _do_ decide that ADVISE AROUND is worth the trouble, here is our
    ;; proposed syntax.  But this actually takes a bit more machinery to
    ;; implement, and offers a greater opportunity for evil.  So we'll try to
    ;; live without it for now.
    
    ;;(def (double x)
    ;;  (.record-event! 'double)
    ;;  (* 2 x))

    ;;(advise around (double x)
    ;;  (.record-event! 'before-double)
    ;;  (let [[result (original x)]]
    ;;    (.record-event! 'after-double)
    ;;    result))

    (def (not-overridden)
      (.record-event! 'not-overridden))
    )

  (define-class %another-advised% (%advised%)
    (def (foo)
      (.record-event! 'another-foo-1)
      (super)
      (.record-event! 'another-foo-2))
    (advise before (foo)
      (.record-event! 'before-another-foo))
    (advise after (foo)
      (.record-event! 'after-another-foo))
    
    (advise after (not-overridden)
      (.record-event! 'after-not-overridden))
    )
  
  (define-test-case <advise-test> () []
    (test "ADVISE should run code before and after the original method"
      (define advised (%advised% .new))
      (assert-equals '() (advised .events))
      (advised .foo)
      (assert-equals '(before-foo foo after-foo) (advised .events)))
    
    (test "ADVISE should not affect the return value"
      (define advised (%advised% .new))
      (assert-equals 'foo-result (advised .foo)))
    
    (test "ADVISE should interleave with method calls when using SUPER"
      (define another (%another-advised% .new))
      (another .foo)
      (assert-equals '(before-another-foo another-foo-1
                       before-foo foo after-foo
                       another-foo-2 after-another-foo)
                     (another .events)))
    
    (test "ADVISE AFTER should not pass SUPER to advice"
      (define advised (%advised% .new))
      (assert-raises exn:fail? (advised .call-super-from-advice)))
    
    (test "ADVISE AFTER should work in the absense of an actual method"
      (define advised (%advised% .new))
      (advised .not-overridden)
      (assert-equals '(not-overridden) (advised .events))
      
      (define another (%another-advised% .new))
      (another .not-overridden)
      (assert-equals '(not-overridden after-not-overridden) (another .events)))
    )
  
  (card ruby-objects-test/advise-test (%test-suite%)
    (value tests (list <advise-test>)))


  ;;=======================================================================
  ;;  Initialize without SUPER
  ;;=======================================================================
  ;;  This used to be a nasty little bug where we threw an infinite number
  ;;  of errors while trying to handle the previous error.
  
  (define-class %initialize-without-super% (%custom-element%)
    (def (initialize &rest args)
      (debug-log "Calling the bad initialize method.")))
  
  (define-test-case <initialize-without-super-test> () []
    (test-elements "Failing to call SUPER shouldn't cause an infinite loop"
      (assert-raises exn:fail?
                     (%initialize-without-super% .new
                        :name 'bad-init
                        :at (point 0 0)
                        :shape (shape 200 300)))))
  
  (card ruby-objects-test/initialize-without-super-test (%test-suite%)
    (value tests (list <initialize-without-super-test>)))
  
  
  ;;=======================================================================
  ;;  Test for good error messages
  ;;=======================================================================
  
  (define-class %error-message-test% ()
    (attr attribute 10 :writable? #t)
    (def (one-arg bar)
      (cat "hello " bar))
    (def (zero-arg)
      "Zonk!")
    (def (rest-args one two &rest rest)
      (cat "The args are: " one two rest)))
  
  (define-test-case <error-message-test> () []
    (test "Error messages should mention the method that failed"
      (define test (%error-message-test% .new))
      (assert-raises-message exn:fail?
        "one-arg" 
        (test .one-arg)))
    (test "Error messages should mention user-visible arity, not internal"
      (define test (%error-message-test% .new))
      (assert-raises-message exn:fail?
        "expects 1 argument, given 0" 
        (test .one-arg))
      (assert-raises-message exn:fail?
        "expects no arguments, given 1"
        (test .zero-arg 10))
      (assert-raises-message exn:fail?
        "expects at least 2 arguments, given 1"
        (test .rest-args "hi"))
      (assert-raises-message exn:fail?
        "expects 1 argument, given 3"
        (test .set-attribute! 'a 2 "foo"))
      (assert-raises-message exn:fail?
        "expects 1 argument, given 0"
        (test .set-attribute!)))
    (test "Error messages should mention class name"
      (define test (%error-message-test% .new))
      (assert-raises-message exn:fail?
        "%error-message-test%" 
        (test .one-arg)))
    (test "Errors accessing attributes should mention attribute names"
      (define test (%error-message-test% .new))
      (assert-raises-message exn:fail?
        "attribute"
        (test .attribute 20))))
  
  (card ruby-objects-test/error-message-test (%test-suite%)
    (value tests (list <error-message-test>)))
  
  )
