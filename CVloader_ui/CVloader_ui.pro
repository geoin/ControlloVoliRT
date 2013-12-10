HEADERS = droparea.h \
          dropwindow.h
SOURCES = droparea.cpp \
          dropwindow.cpp \
          CVloader_ui.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/draganddrop/dropsite
sources.files = $$SOURCES $$HEADERS $$RESOURCES *.pro images
sources.path = $$[QT_INSTALL_EXAMPLES]/draganddrop/dropsite
INSTALLS += target sources
