(module updater (lib "5L.ss" "5L")
  (require (lib "tamale.ss" "5L"))

  ;; TODO - these should probably be factored out into some sort of file-utils
  ;; library.
  (provide dir-writeable? root-directory-writeable? delete-directory-recursive 
           copy-recursive read-string-from-file)
  
  ;; TODO - should probably be moved to tamale.ss, and used in 
  ;; ensure-dir-exists. Also, is current-directory really the right way
  ;; to get the root directory? 
  (define (root-directory-writeable?)
    (dir-writeable? (current-directory)))
  
  (define (dir-writeable? dir)
    (memq 'write (file-or-directory-permissions dir)))
  
  (define (delete-directory-recursive path)
    (cond 
      [(link-exists? path) (delete-file path)]
      [(directory-exists? path) 
       (foreach [inner (directory-list path)]
         (delete-directory-recursive (build-path path inner)))
       (delete-directory path)]
      [(file-exists? path) (delete-file path)]))
  
  (define (strip-base path)
    (let-values [[[base file must-be-dir] (split-path path)]]
      file))
  
  ;; Copy a file, or recursively copy a directory and all contained files and 
  ;; directories. Is designed to emulate cp -r, so it copies links rather than
  ;; indirecting through them, and will not overwrite an existing directory 
  ;; but instead will copy into it. 
  (define (copy-recursive src dest)
    (if (directory-exists? dest)
      (copy-recursive src (build-path dest (strip-base src)))
      (begin
        (cond 
          [(link-exists? src) (copy-file src dest)]
          [(directory-exists? src)
           (make-directory dest)
           (foreach [file (directory-list src)]
             (copy-recursive (build-path src file) (build-path dest file)))]
          [(file-exists? src) (copy-file src dest)])
        dest)))
  
  (define (read-string-from-file file)
    (with-input-from-file 
     file 
     (thunk (let loop [[str ""] [last ""]]
              (if (eof-object? last)
                str
                (loop (string-append str last) (read-string 100)))))))
  
  ;;===========================================================================
  
  (provide <downloader> <mock-downloader> add-mock-url download 
           mock-downloader-from-dir
           parse-manifest add-urls-from-manifests parse-spec-file)
  
  (defclass <downloader> ()
    directory)
  
  (defclass <mock-downloader> (<downloader>)
    [files :initvalue '()])
  
  (define (add-mock-url mock-downloader url file)
    (push! (cons url file) (mock-downloader-files mock-downloader)))
  
  (define (mock-downloader-from-dir dir &key prefix download-dir)
    (define downloader (make <mock-downloader> :directory download-dir))
    (add-mock-urls-recursive downloader prefix dir)
    downloader)
  
  (define manifest-digest first)
  (define manifest-size second)
  (define manifest-file third)
  (define build-url cat)
  
  (define (add-urls-from-manifests downloader prefix dir)
    (foreach (file (parse-manifests-in-dir dir))
      (add-mock-url 
       downloader 
       (build-url prefix (manifest-digest file))
       (read-string-from-file (build-path dir (manifest-file file))))))
  
  (define (add-mock-urls-recursive downloader url path)
    (cond 
      [(link-exists? path) #f] 
      [(directory-exists? path) 
       (foreach [file (directory-list path)]
         (add-mock-urls-recursive
          downloader (cat url file) (build-path path file)))]
      [(file-exists? path) (add-mock-url downloader url
                                         (read-string-from-file path))]))

  ;; SECURITY WARNING - URLs parsed by last-component are not fully sanitized, 
  ;; and so could be used to overwrite arbitrary files. Basic sanitation is 
  ;; done to prevent basic directory traversal attacks, but there may be other 
  ;; attacks possible.
  ;; TODO - should change this to a positive match (like [A-Za-z0-9._-]) 
  ;; instead of a negative match. 
  (define (last-component url)
    (let [[results (regexp-match (regexp "/([^/\\\\]+)(#|$)") url)]]
      (if results
        (second results)
        results)))
  
  (define (next-temp-file-in-dir dir)
    (define max-num
      (foldl (fn (file cur-max) 
               (let ((match (regexp-match (regexp "^temp([0-9]+)$") file)))
                 (if match
                   (max cur-max (read-from-string (second match)))
                   cur-max)))
             0
             (directory-list dir)))
    (cat "temp" (1+ max-num)))
         
  (defgeneric download-file (downloader url file))
  
  (defmethod download-file ((downloader <mock-downloader>) url file)
    (with-output-to-file 
     file
     (thunk 
       (define content (assoc url (mock-downloader-files downloader)))
       (if content
         (display (cdr content))
         (error (cat "URL not found: " url))))))
  
  (defmethod download-file ((downloader <downloader>) url file)
    (call-5l-prim 'Download url file))
  
  (define (download downloader url &key (file #f))
    (define path 
      (build-path 
       (downloader-directory downloader)  
       (or file (last-component url)
           (next-temp-file-in-dir (downloader-directory downloader)))))
    (download-file downloader url path))
  
  (define (parse-manifest path)
    (with-input-from-file 
     path
     (thunk
       (parse-opened-manifest))))
  
  (define (parse-opened-manifest)
    (define r (regexp "^([0-9a-fA-F]*) ([0-9]+) (.+)$"))
    (let loop [[l '()]]
      (let [[line (read-line)]]
        (if (eof-object? line)
          (reverse l)
          (let [[m (regexp-match r line)]]
            (if m
              (loop (cons (list (second m) ; The hash
                                (string->number (third m)) ; The size
                                (fourth m)) ; The filename
                          l))))))))
  
  (define (parse-spec-file path)
    (with-input-from-file
     path
     (thunk
       (parse-opened-spec-file))))
  
  (define (parse-opened-spec-file)
    (define r (regexp "^([a-zA-Z0-9-]*): *(.*)$"))
    (let loop [[alist '()]]
      (let [[line (read-line)]]
        (cond
          ((eof-object? line) alist)
          ((= (string-length line) 0) 
           (cons (list "MANIFEST" (parse-opened-manifest)) alist))
          (else (let [[m (regexp-match r line)]]
                  (if m
                    (loop (cons (cdr m) alist)))))))))
  
  ;;========================================================================
  
  (provide auto-update-possible?
           check-for-update
           download-update
           apply-update
           init-updater! 
           clear-updater!)
  
  (define (spec-file-get alist key)
    (define entry (assoc key alist))
    (unless entry (error "Corrupt .spec file" key alist))
    (second entry))
  
  (define (auto-update-possible?) 
    (and (dir-writeable? (updater-root-directory *updater*))
         (file-exists? 
          (build-path (updater-root-directory *updater*) "release.spec"))))
  
  (defclass <spec> ()
    url build meta-manifest)
  
  (define (read-spec file)
    (define raw (parse-spec-file file))
    (make <spec> 
          :url (spec-file-get raw "Update-URL")
          :build (spec-file-get raw "Build")
          :meta-manifest (spec-file-get raw "MANIFEST")))
  
  (defclass <updater> () 
    root-directory downloader staging? base-spec update-spec url-prefix)
  
  (define *updater* #f)
  
  (define (ensure-dir-exists-absolute dir)
    (when (not (directory-exists? dir))
      (make-directory dir))
    dir)
  
  ;; TODO - work out dependencies between auto-update-possible and 
  ;; init-updater!, so I can make sure that updates are possible when 
  ;; I do init. 
  (define (init-updater! &key (root-directory #f) (staging? #f))
    (define root-dir (or root-directory (current-directory)))
    (define download-dir 
      (ensure-dir-exists-absolute (build-path root-dir "Temp")))
    (define spec (read-spec (build-path root-dir "release.spec")))
    (set! *updater* 
          (make <updater> 
                 :root-directory root-dir
                 :staging? staging?
                 :downloader (make <downloader> :directory download-dir)
                 :base-spec spec
                 :url-prefix (spec-url spec))))
  
  (define (downloaded-file name)
    (build-path (downloader-directory (updater-downloader *updater*)) name))
  
  (define (register-update-spec file)        
    (define spec (read-spec file))
    ;; We change our URL to the one from the spec file we download, so we can
    ;; do simple redirects. 
    ;; TODO - reenable, once I find a better way to force a URL for unit
    ;; testing.
    ; (set! (updater-url-prefix *updater*) (spec-url spec))
    (set! (updater-update-spec *updater*) spec))
  
  (define (clear-updater!) (set! *updater* #f))
  
  (provide set-updater-url!)
  (define (set-updater-url! url) (set! (updater-url-prefix *updater*) url))
  
;  (define *update-downloader* #f)
;  (define *update-root-directory* #f)
  
;  (define (set-update-downloader! val) (set! *update-downloader* val))
;  (define (set-update-root-directory! val) (set! *update-root-directory* val))
  
;  (define (create-and-set-download-dir! downloader)
;    (define dir (build-path *update-root-directory* "Temp"))
;    (unless (directory-exists? dir)
;      (make-directory dir))
;    (set! (downloader-directory downloader) dir)
;    dir)
  
  (define (get-manifest-names dir)
    (define r (regexp "^MANIFEST\\..*$"))
    (filter (fn (file) (regexp-match r file)) (directory-list dir)))
  
  (define (file-hash-from-manifest manifest)
    (define hash (make-hash-table 'equal))
    (foreach (line manifest)
      (hash-table-put! hash (manifest-file line) line))
    hash)
  
  (provide diff-manifests)
  
  ;; TODO - complain if hashes match but sizes differ
  (define (diff-manifests base-manifest update-manifest)
    (define base-files (file-hash-from-manifest base-manifest))
    (define update-files (file-hash-from-manifest update-manifest))
    (define diff '())
    (foreach ((name info) update-files)
      (unless (equal? (hash-table-get base-files name (thunk #f)) info)
        (push! info diff)))
    diff)
  
  (define (parse-manifests-in-dir dir)
    (define manifests (get-manifest-names dir))
    (foldl append '() 
           (map (fn (file) (parse-manifest (build-path dir file))) manifests)))

  (define (manifest-url-prefix)
    (build-url (updater-url-prefix *updater*) 
               "manifests/" 
               (spec-build (updater-update-spec *updater*))
               "/"))
  
  ;; TODO - check if diffs already exist. 
  ;; TODO - should only get manifests that differ in the spec file.
  (provide get-manifest-diffs)
  (define (get-manifest-diffs)
    (define url (manifest-url-prefix))
    (define root-dir (updater-root-directory *updater*))
    (define download-dir (downloader-directory (updater-downloader *updater*)))
    (define base-manifests (get-manifest-names root-dir))
    (foreach (manifest base-manifests)
      ;; TODO - Deal with failures, deal with time issues.
      (download (updater-downloader *updater*) (build-url url manifest)))
    (diff-manifests (parse-manifests-in-dir root-dir)
                    (parse-manifests-in-dir download-dir)))
  
  (define (download-update-spec)
    (define url (cat (updater-url-prefix *updater*)
                     (if (updater-staging? *updater*)
                       "staging.spec"
                       "release.spec")))
    (download (updater-downloader *updater*) url :file "release.spec")
    (assert (file-exists? (downloaded-file "release.spec")))
    (register-update-spec (downloaded-file "release.spec")))
  
  ;; Check to see if an update is available. Returns #t if one is.
  ;; XXX - should only do diffs of manifests that we actually have present 
  ;; on this computer. If media has changed, and we have a web install,
  ;; we don't need to do an update. 
  (define (check-for-update)
    (download-update-spec)
    (not (eq? '() (diff-manifests 
                   (spec-meta-manifest (updater-base-spec *updater*))
                   (spec-meta-manifest (updater-update-spec *updater*))))))
  
  ;; Check the size of the update. This requires downloading the manifests, 
  ;; which are large. This should only be done once we have verified that 
  ;; updates are, indeed, available.
  ;; XXX - write this 
  (define (update-size)
    (define diffs (get-manifest-diffs))
    #f
    )
  
  ;; Downloads a particular update. Takes a progress indicator callback. The 
  ;; progress indicator callback will take two arguments, a percentage and a 
  ;; string that indicates what is currently happening. 
  ;; TODO - implement progress indicator
  ;; TODO - download file with temp name at first, then rename
  ;; TODO - throw error if downloads fail (including wrong contents; how do 
  ;;        we check that? should we hash it, or rely on the size?)
  (define (download-update progress)
    (define diffs (get-manifest-diffs))
    (foreach (file diffs)
      (unless (file-exists? 
               (downloaded-file (manifest-digest file)))
        (download (updater-downloader *updater*) 
                  (build-url (updater-url-prefix *updater*)
                             "pool/"
                             (manifest-digest file))))))
  
  ;; Applies a given update. Will launch an updater, pass it the information 
  ;; needed to apply the update, and quit the program so the updater can do 
  ;; its work. 
  (define (apply-update update)
    #f)
  
  )