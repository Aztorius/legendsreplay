TEMPLATE = subdirs

SUBDIRS += src

SUBDIRS += qhttp

src.depends += qhttp
