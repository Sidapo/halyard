;; @BEGIN_LICENSE
;;
;; Halyard - Multimedia authoring and playback system
;; Copyright 1993-2009 Trustees of Dartmouth College
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

(module url-request (lib "halyard.ss" "halyard")
  (require (lib "kernel.ss" "halyard/private"))
  (require (lib "events.ss" "halyard/private"))

  (provide %url-request%)

  ;;; An asynchronous request to download (or upload to) a URL.  This
  ;;; is actually implemented using libcurl, so it should support (perhaps
  ;;; with a bit of tweaking of the C++ code) a wide variety of protocols.
  (define-class %url-request% (%invisible-element%)
    (value %has-engine-element? #t)

    ;;; The URL to download (or upload to).
    (attr url :type <string>)

    ;;; The HTTP method to use.  For other protocols, such as FTP, use the
    ;;; analogous HTTP method: FTP download -> GET, FTP upload -> PUT, etc.
    ;;; Supported values are 'get and 'post.
    (attr method 'get :type <symbol>)

    ;;; The MIME type to use for the request body, if any.
    (attr request-content-type #f)

    ;;; The data to send in the request body, if any.
    (attr request-body #f)

    ;;; Called when a new chunk of data is received from the server.  The
    ;;; data is provided as (event .data).
    (def (data-received event)
      (void))

    ;;; Called when the transfer is done.  To discover what happened, call
    ;;; (event .success?) and (event .message?).
    (def (transfer-finished event)
      (void))

    ;; Internal.
    (def (create-engine-node)
      (call-prim 'UrlRequest (.full-name)
                 (make-node-event-dispatcher self)
                 (.url))
      (case (.method)
        [[get] (void)]
        [[post]
         (call-prim 'UrlRequestConfigurePost (.full-name)
                    (.request-content-type) (.request-body))]
        [else
         (error (cat self ": Unknown request method " (.method)))])
      (call-prim 'UrlRequestStart (.full-name))
      )
    )

  )