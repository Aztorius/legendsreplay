TEMPLATE = subdirs

SUBDIRS += src

SUBDIRS += qhttp

src.depends += qhttp

QMAKE_CXXFLAGS += -std=c++14
