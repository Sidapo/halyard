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

  (define-class %url-request% (%invisible-element%)
    (default %has-engine-element? #f)

    (attr url :type <string>)

    (def (data-received event)
      (void))

    (def (transfer-finished event)
      (void))

    (def (create-engine-node)
      (call-prim 'UrlRequest (.full-name)
                 (make-node-event-dispatcher self)
                 (.url)))
    )

  )