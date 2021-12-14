;;; plplot-c-style.el --- PLplot's C/C++ style for c-mode

;; Keywords: c, tools

;; The original version of this file before our modifications was
;; google-c-style.el which had the following copyright notice:
;; google-c-style.el is Copyright (C) 2008 Google Inc. All Rights Reserved.
;;
;; It is free software; you can redistribute it and/or modify it under the
;; terms of either:
;;
;; a) the GNU General Public License as published by the Free Software
;; Foundation; either version 1, or (at your option) any later version, or
;;
;; b) the "Artistic License".

;;; Commentary:

;; Provides the PLplot C/C++ coding style. You may wish to add
;; `plplot-set-c-style' to your `c-mode-common-hook' after requiring this
;; file. For example:
;;
;;    (add-hook 'c-mode-common-hook 'plplot-set-c-style)
;;
;; If you want the RETURN key to go to the next line and space over
;; to the right place, add this to your .emacs right after the load-file:
;;
;;    (add-hook 'c-mode-common-hook 'plplot-make-newline-indent)

;;; Code:

(defconst plplot-c-style
  `((c-recognize-knr-p . nil)
    (c-enable-xemacs-performance-kludge-p . t) ; speed up indentation in XEmacs
    (c-basic-offset . 2)
    (indent-tabs-mode . nil)
    (c-comment-only-line-offset . 0)
    (c-hanging-braces-alist . ((defun-open after)
                               (defun-close before after)
                               (class-open after)
                               (class-close before after)
                               (namespace-open after)
                               (inline-open after)
                               (inline-close before after)
                               (block-open after)
                               (block-close . c-snug-do-while)
                               (extern-lang-open after)
                               (extern-lang-close after)
                               (statement-case-open after)
                               (substatement-open after)))
    (c-hanging-colons-alist . ((case-label)
                               (label after)
                               (access-label after)
                               (member-init-intro before)
                               (inher-intro)))
    (c-hanging-semi&comma-criteria
     . (c-semi&comma-no-newlines-for-oneline-inliners
        c-semi&comma-inside-parenlist
        c-semi&comma-no-newlines-before-nonblanks))
    (c-indent-comments-syntactically-p . nil)
    (comment-column . 40)
    (c-cleanup-list . (brace-else-brace
                       brace-elseif-brace
                       brace-catch-brace
                       empty-defun-braces
                       defun-close-semi
                       list-close-comma
                       scope-operator))
    (c-offsets-alist . ((arglist-intro . ++)
                        (func-decl-cont . ++)
                        (member-init-intro . ++)
                        (inher-intro . ++)
                        (comment-intro . 0)
                        (arglist-close . c-lineup-arglist)
                        (topmost-intro . 0)
                        (block-open . 0)
                        (inline-open . 0)
                        (substatement-open . 0)
                        (statement-cont
                         .
                         (,(when (fboundp 'c-no-indent-after-java-annotations)
                             'c-no-indent-after-java-annotations)
                          ,(when (fboundp 'c-lineup-assignments)
                             'c-lineup-assignments)
                          ++))
                        (label . /)
                        (case-label . +)
                        (statement-case-open . +)
                        (statement-case-intro . +) ; case w/o {
                        (access-label . /)
                        (innamespace . 0))))
  "PLplot C/C++ Programming Style")

(defun plplot-set-c-style ()
  "Set the current buffer's c-style to PLplot C/C++ Programming
  Style. Meant to be added to `c-mode-common-hook'."
  (interactive)
  (make-local-variable 'c-tab-always-indent)
  (setq c-tab-always-indent t)
  (c-add-style "PLplot" plplot-c-style t))

(defun plplot-make-newline-indent ()
  "Sets up preferred newline behavior. Not set by default. Meant
  to be added to `c-mode-common-hook'."
  (interactive)
  (define-key c-mode-base-map "\C-m" 'newline-and-indent)
  (define-key c-mode-base-map [ret] 'newline-and-indent))

(provide 'plplot-c-style)
;;; plplot-c-style.el ends here
