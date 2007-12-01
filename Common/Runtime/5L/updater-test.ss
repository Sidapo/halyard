(module updater-test (lib "5l.ss" "5L")
  (require (lib "tamale-unit.ss" "5L"))
  (require (lib "updater.ss" "5L"))
  
  (define vc-exclude (list (regexp "\\.svn$")))
  
  (define (create-test-file dir name)
    (with-output-to-file (build-path dir name)
      (thunk (display name) (newline))))
  
  (define-test-case <filesystem-test> () 
      [[test-directory #f]
       [outer-directory #f]
       [inner-directory #f]]
    (setup
      (set! (test-directory self) (ensure-directory-exists "UpdateTest"))
      (set! (outer-directory self) 
            (ensure-directory-exists (build-path "UpdateTest" "Outer")))
      (set! (inner-directory self) 
            (ensure-directory-exists (build-path "UpdateTest" "Outer" "Inner")))
      (create-test-file (outer-directory self) "Deleted")
      (create-test-file (inner-directory self) "File1")
      (create-test-file (inner-directory self) "File2"))
    (teardown 
      (delete-directory-recursive (test-directory self)))
    ;; Note that we can get away with calling UNSAFE-DIRECTORY-WRITEABLE?
    ;; under Vista in this test case ONLY because we expect to be running
    ;; in a Subversion checkout, not in an installed copy.
    (test "New directory should be writeable."
      (assert (unsafe-directory-writeable? (test-directory self))))
    (test "delete-directory-recursive should delete only the correct files."
      (let ((deleted-dir (outer-directory self))
            (inner-dir (inner-directory self)))
        (create-test-file (test-directory self) "NotDeleted")
        
        ;; Make sure our files exist before the delete.
        (assert (directory-exists? deleted-dir))
        (assert (directory-exists? inner-dir))
        (assert (file-exists? (build-path deleted-dir "Deleted")))
        (assert (file-exists? (build-path inner-dir "File1")))
        (assert (file-exists? (build-path inner-dir "File2")))
        (assert (file-exists? 
                 (build-path (test-directory self) "NotDeleted")))
        
        ;; Delete the files.
        (delete-directory-recursive deleted-dir)
        
        ;; Make sure the files don't exist, except for the file outside
        ;; the deleted directory. 
        (assert (not (directory-exists? deleted-dir)))
        (assert (not (directory-exists? inner-dir)))
        (assert (not (file-exists? (build-path deleted-dir "Deleted"))))
        (assert (not (file-exists? (build-path inner-dir "File1"))))
        (assert (not (file-exists? (build-path inner-dir "File2"))))
        (assert (file-exists? 
                 (build-path (test-directory self) "NotDeleted")))))
    (test "copy-recursive should copy correct files."
      (define copy-dir (build-path (test-directory self) "Copy"))
      (define inner-copy-dir (build-path copy-dir "Inner"))
      (copy-recursive-excluding (list (regexp "Deleted")) 
                                (outer-directory self) copy-dir)
      (assert (directory-exists? copy-dir))
      (assert (directory-exists? inner-copy-dir))
      (assert (not (file-exists? (build-path copy-dir "Deleted"))))
      (assert (file-exists? (build-path inner-copy-dir "File1")))
      (assert (file-exists? (build-path inner-copy-dir "File2")))))
  
  ;; TODO - possibly refactor these into some sort of file-utils and 
  ;; tamale-unit. 

  (define (assert-file-equals str file)
    (if (file-exists? file)
      (let ((contents (read-string-from-file file)))
        (unless (equal? str (read-string-from-file file))
          (error 
           (cat "Expected <" str "> in file <" file ">, got <" contents ">"))))
      (error (cat "File <" file "> does not exist"))))
  
  ;; NOTE - this is dead code. It's just here until I refactor the 
  ;; sanitization tests out. 
  (define-test-case <mock-downloader-test> () 
      [[test-directory #f]
       [downloader #f]]
    (setup 
      (set! (test-directory self) (ensure-directory-exists "MockTest"))
      (set! (downloader self) 
            (make <mock-downloader> :directory (test-directory self)))
      (add-mock-url (downloader self) "test://foo.com/bar" "foo\nbar\n")
      (add-mock-url (downloader self) "test://example.com/example" "example")
      (add-mock-url (downloader self) "test://evil.com/..\\BADFILE" "bad!")
      (add-mock-url (downloader self) "test://nofilename.com/" "noname"))
    (teardown 
      (delete-directory-recursive (test-directory self)))
    (test "mock-downloader should write files."
      (download (downloader self) "test://example.com/example")
      (download (downloader self) "test://foo.com/bar" :file "foobar")
      (assert-file-equals "example" 
                          (build-path (test-directory self) "example"))
      (assert-file-equals "foo\nbar\n" 
                          (build-path (test-directory self) "foobar")))
    (test "mock-downloader should do basic sanitization"
      (download (downloader self) "test://evil.com/..\\BADFILE")
      (download (downloader self) "test://nofilename.com/")
      (assert (not (file-exists? (build-path (current-directory) "BADFILE"))))
      (assert-file-equals "bad!" (build-path (test-directory self) "temp1"))
      (assert-file-equals "noname" 
                          (build-path (test-directory self) "temp2"))))
  
  (define-test-case <downloader-test> ()
      [[test-directory #f]
       [downloader #f]
       [url-prefix #f]]
    (setup
      (set! (test-directory self) (ensure-directory-exists "DownloadTest"))
      (set! (url-prefix self) 
            (cat "file:///" (path->string (fixture-dir "updater")) 
                 "/downloader/")))
    (teardown
      (delete-directory-recursive (test-directory self)))
    (test "downloader should write files."
      (download (cat (url-prefix self) "test") (test-directory self))
      (download (cat (url-prefix self) "bar") (test-directory self) 
                :name "foobar")
      (assert-file-equals "test\n"
                          (build-path (test-directory self) "test"))
      (assert-file-equals "foo\nbar\n"
                          (build-path (test-directory self) "foobar"))))
  
  (define-test-case <parsing-test> ()
      [[base-directory (build-path (fixture-dir "updater") "base")]]
    (test "Parsing manifests."
      (assert-equals 
       `((,null-digest 0 "bar.txt")
         (,null-digest 0 "foo.txt"))
       (parse-manifest (build-path (base-directory self) "MANIFEST.base")))
      (assert-equals 
       `((,null-digest 0 "sub/baz.txt")
         (,null-digest 0 "sub/foo.txt"))
       (parse-manifest (build-path (base-directory self) "MANIFEST.sub"))))
    (test "Parsing .spec files."
      (assert-set-equal 
       '(("Update-URL" "http://www.example.com/updates/")
         ("Build" "base")
         ("MANIFEST" (("142b5a7005ee1b9dc5f1cc2ec329acd0ad3cc9f6"
                       110 "MANIFEST.sub")
                      ("82b90fb155029800cd45f08d32df240d672dfd5b"
                       102 "MANIFEST.base"))))
       (parse-spec-file (build-path (base-directory self) "release.spec")))))
  
  (define null-digest "da39a3ee5e6b4b0d3255bfef95601890afd80709")
  (define foo-digest "855426068ee8939df6bce2c2c4b1e7346532a133")
  
  ;; TODO - macroize?, move to tamale-unit
  (define (assert-set-equal a b)
    (foreach (item a)
      (unless (member? item b)
        (error (cat "Couldn't find " item " in " b))))
    (foreach (item b)
      (unless (member? item a)
        (error (cat "Couldn't find " item " in " a)))))

  ;; Construct a URL to one of our fake update servers.
  (define (update-server-url name)
    (cat "file:///" (path->string (fixture-dir "updater")) "/" name "/"))

  ;; TODO - add test case for update spec file having new URL. 
  (define-test-case <updater-test> () 
      [[test-directory #f]
       [base-directory #f]
       [update-directory #f]
       [url-prefix #f]]
    (setup 
      (set! (test-directory self) (ensure-directory-exists "UpdaterTest"))
      (set! (base-directory self)
            (copy-recursive-excluding vc-exclude 
             (build-path (fixture-dir "updater") "base") 
             (test-directory self)))
      (set! (update-directory self)
            (copy-recursive-excluding vc-exclude 
             (build-path (fixture-dir "updater") "update")
             (test-directory self)))
      (set! (url-prefix self) (update-server-url "update-server")))
    (teardown 
      (delete-directory-recursive (test-directory self))
      (clear-updater!))
    (test "diff-manifests should work."
      (define manifest-a '(("123" 0 "foo.txt")
                           ("456" 1 "bar.txt")
                           ("ABC" 2 "sub/thing.txt")))
      (define manifest-b '(("125" 2 "foo.txt")
                           ("456" 1 "bar.txt")
                           ("ABC" 2 "sub/thing.txt")
                           ("DEF" 3 "sub/zot.txt")))
      (assert-set-equal '() (diff-manifests manifest-a manifest-a))
      (assert-set-equal '(("125" 2 "foo.txt") ("DEF" 3 "sub/zot.txt")) 
                        (diff-manifests manifest-a manifest-b)))
    (test "Automatic update should be possible." 
      (assert (auto-update-possible? (base-directory self)))
      (init-updater! :root-directory (base-directory self)))
    (test "Checking for staging update, update should be available."
      (assert (auto-update-possible? (base-directory self)))
      (init-updater! :root-directory (base-directory self) :staging? #t)
      (set-updater-url! (url-prefix self))
      (assert (check-for-update))
      (assert (not (null? (get-manifest-diffs))))
      (assert-equals 5 (update-size)))
    (test "Checking for staging update, update should not be available."
      (assert (auto-update-possible? (update-directory self)))
      (init-updater! :root-directory (update-directory self) :staging? #t)
      (set-updater-url! (url-prefix self))
      (assert (not (check-for-update)))
      (assert-equals '() (get-manifest-diffs)))
    (test "Checking for regular update, update should not be available."
      (assert (auto-update-possible? (base-directory self)))
      (init-updater! :root-directory (base-directory self))
      (set-updater-url! (url-prefix self))
      (assert (not (check-for-update)))
      (assert-equals '() (get-manifest-diffs)))
    (test "Checking for downgrade, update should be available."
      (assert (auto-update-possible? (update-directory self)))
      (init-updater! :root-directory (update-directory self))
      (set-updater-url! (url-prefix self))
      (assert (check-for-update))
      (assert (not (null? (get-manifest-diffs))))
      (assert-equals 0 (update-size)))
    (test "Downloading files for update."
      (assert (auto-update-possible? (base-directory self)))
      (init-updater! :root-directory (base-directory self) :staging? #t)
      (set-updater-url! (url-prefix self))
      (assert (check-for-update))
      (define download-dir (build-path (base-directory self) "Updates"))
      (define callback-args '())
          
      (download-update (fn (a b) (push! (list a b) callback-args)))
          
      (assert-set-equal '("release.spec" "release.spec.sig"
                          "manifests" "pool" "temp")
                        (map path->string (directory-list download-dir)))
      ;; TODO - this is actually not optimal, since we already have a file 
      ;; that hashes to null-digest on our machine. A future optimization 
      ;; may simply copy any files we know we have, instead of downloading 
      ;; them. This would be a big win if we did a reorg of media.
      (assert-set-equal 
       (list foo-digest null-digest)
       (map path->string 
            (directory-list (build-path download-dir "pool"))))
      (assert-set-equal 
       '("MANIFEST-DIFF") 
       (map path->string
            (directory-list (build-path download-dir "temp"))))
      ;; TODO - test contents of MANIFEST-DIFF
      (assert-set-equal 
       '("update") 
       (map path->string 
            (directory-list (build-path download-dir "manifests"))))
      (assert-set-equal '("MANIFEST.base" "MANIFEST.sub")
                        (map path->string
                             (directory-list 
                              (build-path download-dir "manifests" "Update"))))
      (assert-file-equals "foo\r\n" 
                          (build-path download-dir "pool" foo-digest))
      (assert-file-equals "" (build-path download-dir "pool" null-digest))
          
      ;; XXX - this is an unstable test. Because there are two files that 
      ;; are being updated to "foo\r\n", we might be reporting either one
      ;; of them as the one we download. Also, we might be doing this in 
      ;; either order.
      (assert-set-equal '((0 "foo.txt") 
                          (1 "foo.txt") 
                          (1 "sub/quux.txt")
                          (1 "sub/quux.txt")) callback-args))

    (test "Updater should check signature on *.spec file"
      (assert (auto-update-possible? (base-directory self)))
      (init-updater! :root-directory (base-directory self) :staging? #t)
      (set-updater-url! (update-server-url "update-server-bad-sig"))
      (assert-raises exn:fail? (check-for-update)))

    ;;(test "Update should fail if manifest has been modified"
    ;;  (assert (auto-update-possible? (base-directory self)))
    ;;  (init-updater! :root-directory (base-directory self) :staging? #t)
    ;;  (set-updater-url! (update-server-url "update-server-bad-manifest"))
    ;;  (assert (check-for-update))
    ;;  (assert-raises exn:fail? (download-update (fn (a b) (void)))))

    ;;(test "Update should fail if file has been modified"
    ;;  (assert (auto-update-possible? (base-directory self)))
    ;;  (init-updater! :root-directory (base-directory self) :staging? #t)
    ;;  (set-updater-url! (update-server-url "update-server-bad-manifest"))
    ;;  (assert (check-for-update))
    ;;  (assert-raises exn:fail? (download-update (fn (a b) (void)))))

    )
  
  (define (sig-dir)
    (build-path (fixture-dir "updater") "signatures"))

  (define (sig-file name)
    (build-path (sig-dir) name))

  (define-test-case <crypto-test> () []
    (test "Good signatures pass validation"
      (assert (gpg-signature-valid? (sig-dir)
                                    (sig-file "good.txt.sig")
                                    (sig-file "good.txt"))))
    (test "Invalid signatures fail validation"
      (assert (not (gpg-signature-valid? (sig-dir)
                                         (sig-file "bad1.txt.sig")
                                         (sig-file "bad1.txt")))))
    (test "Signatures from unknown keys fail validation"
      (assert (not (gpg-signature-valid? (sig-dir)
                                         (sig-file "bad2.txt.sig")
                                         (sig-file "bad2.txt")))))
    (test "SHA 1 hashes for files should be correct"
      (assert-equals "7a499dc7e5c2237b6e85de0f9cada0aa1af0060a"
                     (sha1-file (sig-file "good.txt"))))
    )

  ;; For testing the installer:
  (provide create-installer-fixture)
  (define (create-installer-fixture)
    (define fix-dir (ensure-directory-exists "InstallerFixture"))
    (define base-dir (copy-recursive-excluding vc-exclude 
                      (build-path (fixture-dir "updater") "base") 
                      fix-dir))
    (define url-prefix 
      (cat "file:///" (fixture-dir "updater") "/update-server/"))
    (init-updater! :root-directory base-dir :staging? #t)
    (set-updater-url! url-prefix)
    (assert (auto-update-possible?))
    (assert (check-for-update))
    (download-update (fn (a b) #f)))
  
  (card updater-test
      (%test-suite%
       :tests (list <filesystem-test> <downloader-test> <parsing-test> 
                    <updater-test> <crypto-test>)))
  )