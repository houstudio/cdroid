This file gives cookbooks for using emacs modes to help with editing and
maintaining a consistent style for our various kinds of files.

----------------------------------------------

* CMake files

There is a cmake mode available for emacs. On some systems (the latest
Debian unstable for instance) this may be automatically installed along
with cmake. If not, then the following cookbook should work.

1) Get the cmake-mode.el file from the cmake site
   http://www.cmake.org/CMakeDocs/cmake-mode.el . Note the current file
   has a couple of lines from diff / patch in (starting with >>> or <<<)
   which need removing.

2) Install cmake-mode.el in a suitable location in your system / home
   directory.

3) Add the following to your .emacs file to automatically load cmake
   mode for .cmake and CMakeLists.txt files.

----

;--------------------
; Enable CMake mode |
;--------------------
(setq load-path (cons (expand-file-name "~/lib/site-lisp") load-path))
(autoload 'cmake-mode "CMAKE" "Major mode for editing CMake listfiles")
(setq auto-mode-alist
      (append '(("CMakeLists\\.txt\\'" . cmake-mode)
                ("\\.cmake\\'" . cmake-mode))
              auto-mode-alist))

----

  You will need to alter the directory path depending on where you
  installed cmake-mode.el

4) Check the file works by loading one of the plplot cmake files into
   emacs. You should see CMAKE at the bottom of the screen as the major
   mode.

5) For my version of emacs this does not autoload the helper function
   for unscreamifying the buffer (i.e. all cmake commands in lower case).
   Typing "M-x load-library cmake-mode" should fix this.

   The two most useful functions are
   "M-x unscreamify-buffer" which will put all cmake commands into
   lower case
   "M-x indent-region" which will indent the selected region
   correctly

----------------------------------------------
DocBook files in doc/docbook/src

There is a well-established  nxml mode for emacs that should be
automatically available for all recent versions of emacs that is useful
for editing our DocBook files.

1) Add the following to your .emacs file to automatically load nxml
   mode for our DocBook files.

----

;--------------------
; Support nxml mode according to /usr/share/doc/nxml-mode/README
;--------------------
(load "/usr/share/emacs/site-lisp/nxml-mode/rng-auto.el")
(setq auto-mode-alist
  (cons '("\\.\\(xml\\|xml.in\\|xsl\\|rng\\|xhtml\\)\\'" . nxml-mode)
    auto-mode-alist))

----

2) Edit one of the DocBook files in doc/docbook/src.

  i) Recognition of the DocBook schema for our DocBook files.

    nxml automatically uses schemas.xml in the above directory to
    use the DocBook schema for our DocBook files.  If we add new DocBook
    files, schemas.xml will have to be changed accordingly.  To confirm you
    are using the correct schema use the

    C-c-C-s-C-w

    emacs command.

  ii) Validation.

    nxml dynamically validates the file as it is being edited and underlines
    in red (with a pop-up explaining the problem if you hover your mouse
    over the underlined element) all elements that it cannot validate.  This
    facility is extremely useful while you edit our DocBook documentation.
    Note, one nxml validation limitation is it does not understand xml
    projects split into more than one file.  In practice, this means all the
    entities defined in plplotdoc.xml(.in) will not validate correctly
    (i.e., will be underlined in red).  Of course, to validate the entire
    PLplot DocBook documentation project including entities, it is important
    to run "make validate" in the build tree after your editing session is
    complete.  To find the next invalid element use the

    C-c-C-n

    emacs command.

  iii) Standard indentation style.

    To impose a standard indentation style on our DocBook files, select a
    region by marking a location with the

    C-@

    emacs command, then moving the point (cursor) to the end of the region you
    want to select.  Then impose a standard indentation style on the
    selected region using the

    C-M-\

    emacs command.  This has recently been done for the entire regions of
    all DocBook files so the region you normally select would normally be
    just the one where you have recently made changes.

  iv) Other nxml possibilities.

    See "XML document authoring with emacs nxml-mode" at
    http://infohost.nmt.edu/tcc/help/pubs/nxml/index.html .

----------------------------------------------
