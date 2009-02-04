;; @BEGIN_LICENSE
;;
;; Mizzen - Scheme object system
;; Copyright 2006-2009 Trustees of Dartmouth College
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

;;; A command-line callable unit-test driver.  You can run this from
;;; mzscheme by typing:
;;;
;;;   PLTCOLLECTS=:/path/to/halyard/test/Runtime
;;;   mzscheme -u test-driver.ss
(module test-driver "mizzen.ss"

  (require "mizzen-unit.ss" "tests.ss")

  (printf "Running unit tests~n")
  (run-tests $mizzen-tests)

  )