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

(module jump-to-each-card (lib "mizzen.ss" "mizzen")
  (require (lib "kernel.ss" "halyard/private"))
  (require (lib "api.ss" "halyard/private"))
  (require (lib "metadata-attr.ss" "halyard"))
  (require (lib "electric-gibbon.ss" "halyard"))
  
  (provide jump-to-each-card)

  (with-instance %group-member%
    ;;; Should we test this %group-member% using jump-to-each-card?  To
    ;;; disable, use:
    ;;;
    ;;;   (group /foo (:skip-when-jumping-to-each-card? #t)
    ;;;     ..)
    (metadata-attr skip-when-jumping-to-each-card? #f))

  ;; Return a list containing all the cards in GROUP-MEM.  This walks the
  ;; node tree recursively.
  (define (all-cards-in group-mem)
    (cond
     [(group-mem .skip-when-jumping-to-each-card?)
      '()]
     [(group-mem .subclass-of? %card%)
      (list group-mem)]
     [else
      (apply append (map all-cards-in (group-mem .members)))]))

  ;; Return a list of all the cards in the program.
  (define (all-cards)
    (all-cards-in (static-root-node)))

  ;; We create a test planner for each card, and let it decide how to
  ;; handle any card-specific testing.
  (define *test-planner* #f)

  ;; We still need to jump to the cards in this list.
  (define *cards-left-to-jump-to* #f)

  ;; Should we honor the next jump (because it's one of ours), or should we
  ;; try to intercept it?
  (define *should-honor-next-jump?* #f)

  ;; Really perform a jump.  This bypasses our jump interception code.
  (define (jump-for-testing card)
    (set! *should-honor-next-jump?* #t)
    ;; TODO - Temporary print card names until we have code to intercept
    ;; error messages.
    (command-line-message (cat "Card: " (card .full-name)))
    (jump card))

  ;; Jump to the next card in our test sequence.
  (define (continue-jumping-to-each-card)
    ;; If we have a test planner, and it has finished running, get rid of
    ;; it before we do anything else.
    (when (and *test-planner* (*test-planner* .done?))
      (set! *test-planner* #f))
    ;; Figure out what to do next.
    (cond
     ;; If we still have an active test planner, jump back to the associated
     ;; card and let it go again.
     [*test-planner*
      (jump-for-testing (*test-planner* .card))]
     ;; If there are no more cards to jump to, quit.
     [(null? *cards-left-to-jump-to*)
      (exit-script)]
     ;; Jump to the next card that we need to test.
     [else
      (jump-for-testing (pop! *cards-left-to-jump-to*))]))

  ;; Decide whether we should intercept the current jump and replace it
  ;; with a jump to the next card we need to test.
  (define (maybe-intercept-jump)
    (cond
     ;; If this jump is one of ours, clear the flag and continue.
     [*should-honor-next-jump?*
      (set! *should-honor-next-jump?* #f)]
     ;; If there's a custom jump handler running, then this is none of our
     ;; business, because we're presumably running unit tests.
     [(custom-jump-handler-installed?)
      (void)]
     ;; Otherwise, stick to our original plan.
     [else
      (continue-jumping-to-each-card)]))

  ;; This function is called when we reach the end of a card body.  If we
  ;; don't already have a test planner for this card, it creates one.  It
  ;; then tries to run as many test actions as possible before it has to
  ;; jump to the card again.
  (define (continue-running-test-actions)
    ;; If we don't have a test planner for this card yet, create one.
    (unless *test-planner*
      (set! *test-planner* (%shallow-test-planner% .new)))

    ;; Run test actions for as long as we can.  This may trigger a JUMP at
    ;; any point.  If a JUMP occurs, then we should eventually make it back
    ;; here when CONTINUE-JUMPING-TO-EACH-CARD notices that our test
    ;; planner isn't done yet.  Also, some actions may cause other actions
    ;; to become unavailable, in which case we'll have to jump back to this
    ;; card and try again.
    (while (*test-planner* .run-next-test-action)
      ;; KLUDGE - Clear our deferred thunk queue after each action.  Yes,
      ;; this is an excessive amount of knowledge of kernel.ss.
      (%kernel-check-deferred))

    ;; There's nothing more we can do here, so we pass the buck to a
    ;; higher level of our test driver.
    (continue-jumping-to-each-card))

  ;;; Jump to each card in the program, and then quit.
  (define (jump-to-each-card)
    ;; When we reach the end of a card body, we want to move on to the next
    ;; card automatically.
    (hook-add-function! *card-body-finished-hook* 'jump-to-each-card
                        continue-running-test-actions)

    ;; When a card jumps to another card, we want to intercept that and
    ;; instead jump to the next card in our list.
    (with-instance (%card% .class)
      (advise before (jump)
        (maybe-intercept-jump)))

    ;; We want to visit the cards in backwards order to help flush out bugs.
    (set! *cards-left-to-jump-to* (reverse (all-cards)))
    (continue-jumping-to-each-card))

  )