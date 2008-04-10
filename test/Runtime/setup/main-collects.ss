(module main-collects mzscheme
  (require "dirs.ss")

  (provide path->main-collects-relative
           main-collects-relative->path)

  ;; Historical note: this module is based on the old "plthome.ss"

  ;; The `path->main-collects-relative' and
  ;; `main-collects-relative->path' functions are used to store paths
  ;; that are relative to the main "collects" directory, such as in
  ;; .dep files.  This means that if the plt tree is moved, .dep files
  ;; still work.  It is generally fine if
  ;; `path->main-collects-relative' misses some usages, as long as it
  ;; works when we prepare a distribution tree.  Otherwise, things
  ;; will continue to work fine and .dep files will just contain
  ;; absolute path names.  These functions work on .dep elements:
  ;; either a pathname or a pair with a pathname in its cdr; the
  ;; `path->main-collects-relative' pathname will itself be a pair.

  ;; we need to compare paths to find when something is in the plt
  ;; tree -- this does some basic "normalization" that should work
  ;; fine: getting rid of `.' and `..' (simplify-path) and collapsing
  ;; `//' to `/' (expand-path).  Using `expand-path' also expands `~'
  ;; and `~user', but this should not be a problem in practice.
  (define (simplify-bytes-path bytes)
    (path->bytes (simplify-path (expand-path (bytes->path bytes)))))
  ;; on Windows, turn backslashes to forward slashes
  (define simplify-path*
    (if (eq? 'windows (system-type))
	(lambda (bytes)
          (simplify-bytes-path (regexp-replace* #rx#"\\\\" bytes #"/")))
	simplify-bytes-path))

  (define main-collects-dir/
    (delay (let ([dir (find-collects-dir)])
             (and dir (regexp-replace #rx#"/*$"
                                      (simplify-path* (path->bytes dir))
                                      #"/")))))

  (define (maybe-cdr-op fname f)
    (lambda (x)
      (cond [(and (pair? x) (not (eq? 'collects (car x))))
             (cons (car x) (f (cdr x)))]
            [else (f x)])))

  ;; path->main-collects-relative* : path-or-bytes -> datum-containing-bytes-or-path
  (define (path->main-collects-relative* path)
    (let* ([path (cond [(bytes? path) path]
                       [(path?  path) (path->bytes path)]
                       [else (error 'path->main-collects-relative
                                    "expecting a byte-string, got ~e" path)])]
           [path* (simplify-path* path)]
           [main-collects-dir/ (force main-collects-dir/)]
	   [mcd-len (bytes-length main-collects-dir/)])
      (cond [(and path*
		  mcd-len
                  (> (bytes-length path*) mcd-len)
                  (equal? (subbytes path* 0 mcd-len)
			  main-collects-dir/))
             (cons 'collects (subbytes path* mcd-len))]
            [(equal? path* main-collects-dir/) (cons 'collects #"")]
            [else path])))

  ;; main-collects-relative->path* : datum-containing-bytes-or-path -> path
  (define (main-collects-relative->path* path)
    (cond [(and (pair? path)
                (eq? 'collects (car path))
                (bytes? (cdr path)))
	   (let ([dir (or (find-collects-dir)
			  ;; No main "collects"? Use original working directory:
			  (find-system-path 'orig-dir))])
	     (if (equal? (cdr path) #"")
		 dir
		 (build-path dir (bytes->path (cdr path)))))]
          [(bytes? path) (bytes->path path)]
          [else path]))

  (define path->main-collects-relative
    (maybe-cdr-op 'path->main-collects-relative path->main-collects-relative*))
  (define main-collects-relative->path
    (maybe-cdr-op 'main-collects-relative->path main-collects-relative->path*))
  )