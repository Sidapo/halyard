;; @BEGIN_LICENSE
;;
;; Mizzen - Scheme object system
;; Copyright 2006-2008 Trustees of Dartmouth College
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

(module indent (lib "swindle.ss" "swindle")
  
  (provide define-syntax-indent syntax-indent treat-as-syntax?)

  (define *indent-table* (make-hash-table))

  (define-syntax define-syntax-indent
    (syntax-rules (function)
      [(define-syntax-indent name function)
       (hash-table-put! *indent-table* 'name -1)]
      [(define-syntax-indent name value)
       (hash-table-put! *indent-table* 'name value)]))

  (define (syntax-indent name)
    (hash-table-get *indent-table* name (lambda () 0)))

  (define (treat-as-syntax? name)
    (and (hash-table-get *indent-table* name (lambda () #f))
         #t))

  ;; Scheme R5RS.
  (define-syntax-indent quote function)
  (define-syntax-indent lambda 1)
  (define-syntax-indent if 1)
  (define-syntax-indent set! function)
  (define-syntax-indent cond 0)
  (define-syntax-indent case 1)
  (define-syntax-indent and function)
  (define-syntax-indent or function)
  (define-syntax-indent cond 0)
  (define-syntax-indent let 1) ; Doesn't know about "named let".
  (define-syntax-indent let* 1)
  (define-syntax-indent letrec 1)
  (define-syntax-indent begin 0)
  (define-syntax-indent do 2)
  (define-syntax-indent delay function)
  (define-syntax-indent quasiquote function)
  (define-syntax-indent let-syntax 1)
  (define-syntax-indent letrec-syntax 1)
  (define-syntax-indent syntax-rules 1)
  (define-syntax-indent define 1)
  (define-syntax-indent define-syntax 1)

  ;; From Emacs scheme-mode.
  (define-syntax-indent call-with-input-file 1)
  (define-syntax-indent with-input-from-file 1)
  (define-syntax-indent with-input-from-port 1)
  (define-syntax-indent call-with-output-file 1)
  (define-syntax-indent with-output-to-file 1)
  (define-syntax-indent with-output-to-port 1)
  (define-syntax-indent call-with-values 1) ; r5rs?
  (define-syntax-indent dynamic-wind 0) ; r5rs?

  ;; PLT syntax.
  (define-syntax-indent module 2)
  (define-syntax-indent provide function)
  (define-syntax-indent require 0)
  (define-syntax-indent when 1)
  (define-syntax-indent unless 1)
  (define-syntax-indent syntax-case 2)
  (define-syntax-indent match-let 1)
  (define-syntax-indent with-syntax 1)

  ;; Swindle.
  (define-syntax-indent defclass 2)

  )
