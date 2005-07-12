;;=========================================================================
;;  Kernel and Script Loader
;;=========================================================================
;;  This code loads our runtime kernel and our user scripts.  Both the
;;  kernel and the user scripts are loaded into an isolated namespace,
;;  which we can throw away and recreate in a pristine condition whenever
;;  a developer asks us to reload the currently running script.
;;
;;  You can find similar code for the DrScheme IDE in tool.ss.

(module 5L-Loader mzscheme

  (require #%fivel-engine)

  ;; Make sure the "Runtime" and "Script" directories get searched for
  ;; collections of support modules.
  (current-library-collection-paths
   (list (build-path (current-directory) "Runtime")
         (build-path (current-directory) "Scripts")))
  
  ;; Notify the operating system that we're still alive, and haven't hung.
  (define (heartbeat)
    (when (%call-5l-prim 'haveprimitive 'heartbeat)
      (%call-5l-prim 'heartbeat)))

  ;; Import a function the hard way.  We can't just (require ...) this
  ;; module because we don't set up the collection paths until its
  ;; too late to help.
  (define make-compilation-manager-load/use-compiled-handler
    (dynamic-require '(lib "cm.ss" "mzlib")
                     'make-compilation-manager-load/use-compiled-handler))
  
  ;; There's some sort of context dependence that makes it so we can't just 
  ;; define COMPILE-ZO-WITH-HEARTBEAT out here, probably due to some paths 
  ;; not being set up that MAKE-COMPILATION-MANAGER-LOAD/USE-COMPILED-HANDLER
  ;; uses, so instead we just wrap this all in a function that will return our
  ;; COMPILE-ZO-WITH-HEARTBEAT function, and call it at the appropriate time.
  (define (make-compile-zo-with-heartbeat)
    ;; A suitable function to use with CURRENT-LOAD/USE-COMPILED.  This
    ;; handles automatic compilation of *.zo files for us.
    (define compile-zo (make-compilation-manager-load/use-compiled-handler))
    
    ;; Wrap COMPILE-ZO with two calls to HEARTBEAT, just to let the operating
    ;; system know we're still alive during really long loads.
    (define (compile-zo-with-heartbeat file-path expected-module-name)
      (heartbeat)
      (let [[result (compile-zo file-path expected-module-name)]]
        (heartbeat)
        result))
    
    compile-zo-with-heartbeat)

  ;;; The default namespace into which this script was loaded.  We don't
  ;;; use it to run much except this code.
  (define *original-namespace* #f)

  ;;; The namespace in which we're running the currently active script.
  ;;; This will be overwritten if we reload a script.
  (define *script-namespace* #f)

  ;;; Create a new, pristine namespace in which to run the user's 5L
  ;;; script, and install it as the current-namespace parameter for this
  ;;; thread.
  (define (new-script-environment)
    
    ;; Store our original namespace for safe keeping.
    (unless *original-namespace*
      (set! *original-namespace* (current-namespace)))
    
    ;; Create a new, independent namespace and make it the default for all
    ;; code loaded into this thread.
    (set! *script-namespace* (make-namespace 'empty))
    (current-namespace *script-namespace*)
    
    ;; Alias some basic runtime support modules into our new namespace.  This
    ;; tehcnically means that these modules are shared between the original
    ;; namespace and the script namespace, which is fairly weird.
    (namespace-attach-module *original-namespace* 'mzscheme)
    (namespace-attach-module *original-namespace* '#%fivel-engine)
    #f)

  ;;; Call this function to load the 5L kernel and begin running a user
  ;;; script.  Call this immediately *after* calling
  ;;; new-script-environment.
  ;;;
  ;;; @return #f if the load succeeds, or an error string.
  (define (load-script)
    
    ;; Set up an error-handling context so we can report load-time errors
    ;; meaningfully.
    (let ((filename "none"))
      (with-handlers [[void (lambda (exn)
                              (string-append "Error while loading <" filename
                                             ">: " (exn-message exn)))]]

        ;; First, we install enough of a namespace to parse 'module'.
        ;; We'll need this in just a second when we call load/use-compiled.
        (set! filename "bootstrap-env.ss")
        (namespace-require '(lib "bootstrap-env.ss" "5L"))

        ;; Ask MzScheme to transparently compile modules to *.zo files.
        (current-load/use-compiled (make-compile-zo-with-heartbeat))

        ;; Manually load the kernel into our new namespace.  We need
        ;; to call (load/use-compiled ...) instead of (require ...),
        ;; because we want the kernel registered under its official
        ;; module name (so the engine can easily grovel around inside it)
        ;; but not imported into our namespace (which is the job of the
        ;; 5L language module).
        (set! filename "kernel.ss")
        (namespace-require '(lib "kernel.ss" "5L"))
        ;;(load/use-compiled (build-path (current-directory)
        ;;                               "Runtime" "5L"
        ;;                               "kernel.ss"))
      
        ;; Provide a reasonable default language for writing scripts.  We
        ;; need to set up both the transformer environment (which is used
        ;; only by code in macro expanders) and the regular environment
        ;; (which is used by normal program code).
        (set! filename "lispish.ss")
        (namespace-transformer-require '(lib "lispish.ss" "5L"))
        (set! filename "5l.ss")
        (namespace-require '(lib "5l.ss" "5L"))
      
        ;; Load the user's actual script into our new namespace.
        (set! filename (build-path (current-directory) "Scripts" "start.ss"))
        (load/use-compiled filename)
        #f)))

  ) ; end module
