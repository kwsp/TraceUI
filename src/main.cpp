#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

#include "config.h"

int main(int argc, char *argv[]) {
    QQuickWindow::setDefaultAlphaBuffer(true);
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.loadFromModule(APP_MODULE_NAME, "Main");

    return QGuiApplication::exec();
}
