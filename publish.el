;;; publish.el --- Reproducible org-publish config for jingtao.net  -*- lexical-binding: t; -*-
;;
;; Self-contained so the site exports identically anywhere (CI, batch, a fresh
;; checkout) WITHOUT depending on a personal Emacs init.  The old site was
;; exported interactively, which is why the personal nav + gmail footer leaked
;; in from private config.  This file pins:
;;   - HTML5 output, no default inline styles (we ship css/site.css instead)
;;   - a shared nav preamble + footer postamble across every page
;;   - author/email export OFF, so no page can re-inject a personal address
;;   - dev@jingtao.net + github.com/jingtao-net as the only identity
;;
;; Build:   emacs --batch -l publish.el -f org-publish-all
;; Verify:  grep -rIl 'jingtaozf@gmail' --include='*.html' .   # must be empty

(require 'ox-publish)
(require 'ox-html)

(setq user-mail-address "dev@jingtao.net"
      user-full-name "Jingtao Xu")

;; Always re-export every file (so the gmail scrub reaches the old article HTML).
(setq org-publish-use-timestamps-flag nil)

;; Never execute source blocks during export (safe + fast in batch).
(setq org-export-use-babel nil)

(defconst jt-root (file-name-directory (or load-file-name buffer-file-name default-directory)))

(defconst jt-head
  (concat
   "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/>"
   "<link rel=\"stylesheet\" href=\"/css/site.css\"/>"))

(defconst jt-nav
  (concat
   "<nav class=\"site-nav\">"
   "<a class=\"brand\" href=\"/\">Jingtao&nbsp;Xu</a>"
   "<span class=\"nav-links\">"
   "<a href=\"/about/about.html\">About</a>"
   "<a href=\"/#services\">Work</a>"
   "<a href=\"/writing.html\">Writing</a>"
   "<a href=\"/proof.html\">Proof</a>"
   "<a class=\"nav-cta\" href=\"/contact.html\">Connect</a>"
   "</span></nav>"))

(defconst jt-footer
  (concat
   "<footer class=\"site-footer\">"
   "<p>Remote · UTC+8 · fluent English / 中文</p>"
   "<p><a href=\"mailto:dev@jingtao.net\">dev@jingtao.net</a>"
   " · <a href=\"https://cal.com/jingtaoxu\">Book a call</a>"
   " · <a href=\"https://github.com/jingtao-net\">github.com/jingtao-net</a></p>"
   "<p class=\"fineprint\">© Jingtao Xu</p>"
   "</footer>"))

(setq org-html-doctype "html5"
      org-html-html5-fancy t
      org-html-validation-link nil
      org-html-metadata-timestamp-format "%Y-%m-%d")

(setq org-publish-project-alist
      `(("jingtao-pages"
         :base-directory ,jt-root
         :base-extension "org"
         :publishing-directory ,jt-root
         :recursive t
         :publishing-function org-html-publish-to-html
         :html-doctype "html5"
         :html-html5-fancy t
         :html-head-include-default-style nil
         :html-head-include-scripts nil
         :html-head ,jt-head
         :html-preamble ,jt-nav
         :html-postamble ,jt-footer
         :with-author nil
         :with-email nil
         :with-creator nil
         :with-toc nil
         :section-numbers nil
         :html-validation-link nil)))

(provide 'publish)
;;; publish.el ends here
