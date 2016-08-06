TEMPLATE = subdirs

SUBDIRS += src

SUBDIRS += qhttp

src.depends += qhttp

CONFIG += c++14