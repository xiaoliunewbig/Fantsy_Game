/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.3.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../include/ui/windows/MainWindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.3.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Fantasy__MainWindow_t {
    const uint offsetsAndSize[42];
    char stringdata0[296];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_Fantasy__MainWindow_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_Fantasy__MainWindow_t qt_meta_stringdata_Fantasy__MainWindow = {
    {
QT_MOC_LITERAL(0, 19), // "Fantasy::MainWindow"
QT_MOC_LITERAL(20, 11), // "gameStarted"
QT_MOC_LITERAL(32, 0), // ""
QT_MOC_LITERAL(33, 10), // "gamePaused"
QT_MOC_LITERAL(44, 11), // "gameResumed"
QT_MOC_LITERAL(56, 10), // "gameExited"
QT_MOC_LITERAL(67, 15), // "settingsChanged"
QT_MOC_LITERAL(83, 18), // "onGameStateChanged"
QT_MOC_LITERAL(102, 9), // "GameState"
QT_MOC_LITERAL(112, 8), // "newState"
QT_MOC_LITERAL(121, 16), // "onNewGameClicked"
QT_MOC_LITERAL(138, 17), // "onLoadGameClicked"
QT_MOC_LITERAL(156, 17), // "onSettingsClicked"
QT_MOC_LITERAL(174, 13), // "onExitClicked"
QT_MOC_LITERAL(188, 15), // "onResumeClicked"
QT_MOC_LITERAL(204, 23), // "onBackToMainMenuClicked"
QT_MOC_LITERAL(228, 20), // "onWindowStateChanged"
QT_MOC_LITERAL(249, 16), // "Qt::WindowStates"
QT_MOC_LITERAL(266, 5), // "state"
QT_MOC_LITERAL(272, 16), // "onCloseRequested"
QT_MOC_LITERAL(289, 6) // "update"

    },
    "Fantasy::MainWindow\0gameStarted\0\0"
    "gamePaused\0gameResumed\0gameExited\0"
    "settingsChanged\0onGameStateChanged\0"
    "GameState\0newState\0onNewGameClicked\0"
    "onLoadGameClicked\0onSettingsClicked\0"
    "onExitClicked\0onResumeClicked\0"
    "onBackToMainMenuClicked\0onWindowStateChanged\0"
    "Qt::WindowStates\0state\0onCloseRequested\0"
    "update"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Fantasy__MainWindow[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  104,    2, 0x06,    1 /* Public */,
       3,    0,  105,    2, 0x06,    2 /* Public */,
       4,    0,  106,    2, 0x06,    3 /* Public */,
       5,    0,  107,    2, 0x06,    4 /* Public */,
       6,    0,  108,    2, 0x06,    5 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       7,    1,  109,    2, 0x0a,    6 /* Public */,
      10,    0,  112,    2, 0x0a,    8 /* Public */,
      11,    0,  113,    2, 0x0a,    9 /* Public */,
      12,    0,  114,    2, 0x0a,   10 /* Public */,
      13,    0,  115,    2, 0x0a,   11 /* Public */,
      14,    0,  116,    2, 0x0a,   12 /* Public */,
      15,    0,  117,    2, 0x0a,   13 /* Public */,
      16,    1,  118,    2, 0x0a,   14 /* Public */,
      19,    0,  121,    2, 0x0a,   16 /* Public */,
      20,    0,  122,    2, 0x0a,   17 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 17,   18,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Fantasy::MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->gameStarted(); break;
        case 1: _t->gamePaused(); break;
        case 2: _t->gameResumed(); break;
        case 3: _t->gameExited(); break;
        case 4: _t->settingsChanged(); break;
        case 5: _t->onGameStateChanged((*reinterpret_cast< std::add_pointer_t<GameState>>(_a[1]))); break;
        case 6: _t->onNewGameClicked(); break;
        case 7: _t->onLoadGameClicked(); break;
        case 8: _t->onSettingsClicked(); break;
        case 9: _t->onExitClicked(); break;
        case 10: _t->onResumeClicked(); break;
        case 11: _t->onBackToMainMenuClicked(); break;
        case 12: _t->onWindowStateChanged((*reinterpret_cast< std::add_pointer_t<Qt::WindowStates>>(_a[1]))); break;
        case 13: _t->onCloseRequested(); break;
        case 14: _t->update(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MainWindow::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::gameStarted)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::gamePaused)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::gameResumed)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::gameExited)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (MainWindow::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainWindow::settingsChanged)) {
                *result = 4;
                return;
            }
        }
    }
}

const QMetaObject Fantasy::MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_Fantasy__MainWindow.offsetsAndSize,
    qt_meta_data_Fantasy__MainWindow,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_Fantasy__MainWindow_t
, QtPrivate::TypeAndForceComplete<MainWindow, std::true_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>
, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<GameState, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<Qt::WindowStates, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>


>,
    nullptr
} };


const QMetaObject *Fantasy::MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Fantasy::MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Fantasy__MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int Fantasy::MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 15;
    }
    return _id;
}

// SIGNAL 0
void Fantasy::MainWindow::gameStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void Fantasy::MainWindow::gamePaused()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void Fantasy::MainWindow::gameResumed()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void Fantasy::MainWindow::gameExited()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void Fantasy::MainWindow::settingsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
