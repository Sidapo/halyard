;; Various cards that only work when we have a network.  For best results,
;; run tools/http-test-server.rb before experimenting with the cards in
;; this section.
(module networking (lib "halyard.ss" "halyard")
  (require (lib "halyard-unit.ss" "halyard"))
  (require (lib "test-elements.ss" "halyard"))
  (require (lib "url-request.ss" "halyard"))
  (require (lib "base.ss" "halyard-test"))

  (group /networking (%card-group% :skip-when-jumping-to-each-card? #t))


  ;;=======================================================================
  ;;  Demo cards
  ;;=======================================================================
  
  (card /networking/http (%standard-test-card% :title "Asynchronous HTTP")
    (edit-box result
        ((inset-rect $screen-rect 50)
         ""
         :multiline? #t
         :font-size 12))

    (text status ((below (.result) 10) $caption-style "Downloading"))

    (elem req (%url-request% :url "http://iml.dartmouth.edu/halyard")
      (def (display text)
        (set! (.parent.result.text)
              (string-append (.parent.result.text) text)))

      (def (status text)
        (set! (.parent.status.text) text))

      (def (data-received event)
        (.display (regexp-replace* "\r\n" (event .data) "\n")))

      (def (transfer-finished event)
        (.status (cat (if (event .success?)
                          "Transfer succeeded"
                          "Transfer failed")
                      ": " (event .message))))
      )
    )


  ;;=======================================================================
  ;;  Unit tests
  ;;=======================================================================

  (group /networking/tests)

  (define-class %easy-url-request% (%url-request%)
    (attr finished? #f :type <boolean> :writable? #t)
    (attr success? #f :type <boolean> :writable? #t)
    (attr response-body "" :type <string> :writable? #t)

    (def (data-received event)
      (set! (.response-body) (cat (.response-body) (event .data))))

    (def (transfer-finished event)
      (set! (.finished?) #t)
      (set! (.success?) (event .success?)))

    (def (wait-for-response)
      (while (not (.finished?))
        (idle))
      (if (.success?)
        (.response-body)
        #f))
    )

  (define-class %url-request-test-case% (%element-test-case%)
    (def (perform request)
      (define result (request .wait-for-response))
      (delete-element request)
      result)
    (def (get url)
      (.perform
       (%easy-url-request% .new :url (cat "http://localhost:4567" url))))
    (def (post url content-type body)
      (.perform
       (%easy-url-request% .new
         :url (cat "http://localhost:4567" url)
         :method 'post
         :content-type content-type
         :body body)))
    )

  (define-class %test-server-present-test% (%url-request-test-case%)
    (test "tools/http-test-server.rb should be running"
      (assert-equals "Hello!\n" (.get "/hello"))))

  (define-class %http-get-test% (%url-request-test-case%)
    (test "GET should return the response body"
      (assert-equals "Hello!\n" (.get "/hello")))

    (test "GET should support large responses"
      (define (build-hello count)
        ;; Repeat "Hello!\n" COUNT times.  Only works for powers of 2.
        (if (= 1 count)
          "Hello!\n"
          (let [[half (build-hello (/ count 2))]]
            (string-append half half))))
      (assert-equals (build-hello 1024) (.get "/hello?count=1024")))

    (test "GET should fail if it encounters an HTTP error"
      (assert-equals #f (.get "/not-found")))

    (test "GET should provide a correct Content-Type"
      (define request
        (%easy-url-request% .new :url "http://localhost:4567/hello"))
      (request .wait-for-response)
      (assert-equals "text/html" (request .response-content-type)))

    )

  (define-class %http-post-test% (%url-request-test-case%)
    (test "POST should upload data and return the response body"
      (assert-equals "post: Hello" (.post "/upload" "text/plain" "Hello")))
    )

  (card /networking/tests/url-request
      (%test-suite%
       :tests (list %test-server-present-test%
                    %http-get-test%
                    %http-post-test%)))

  )
