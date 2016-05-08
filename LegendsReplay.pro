TEMPLATE = subdirs

SUBDIRS += src

SUBDIRS += deps/qhttp

src.depends += deps/qhttp
