/****************************************************************************
** Meta object code from reading C++ file 'MainMenu.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.3.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../include/ui/components/MainMenu.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainMenu.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.3.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Fantasy__MainMenu_t {
    const uint offsetsAndSize[24];
    char stringdata0[223];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_Fantasy__MainMenu_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_Fantasy__MainMenu_t qt_meta_stringdata_Fantasy__MainMenu = {
    {
QT_MOC_LITERAL(0, 17), // "Fantasy::MainMenu"
QT_MOC_LITERAL(18, 14), // "newGameClicked"
QT_MOC_LITERAL(33, 0), // ""
QT_MOC_LITERAL(34, 15), // "loadGameClicked"
QT_MOC_LITERAL(50, 15), // "settingsClicked"
QT_MOC_LITERAL(66, 11), // "exitClicked"
QT_MOC_LITERAL(78, 22), // "onNewGameButtonClicked"
QT_MOC_LITERAL(101, 23), // "onLoadGameButtonClicked"
QT_MOC_LITERAL(125, 23), // "onSettingsButtonClicked"
QT_MOC_LITERAL(149, 19), // "onExitButtonClicked"
QT_MOC_LITERAL(169, 25), // "onButtonAnimationFinished"
QT_MOC_LITERAL(195, 27) // "onBackgroundAnimationUpdate"

    },
    "Fantasy::MainMenu\0newGameClicked\0\0"
    "loadGameClicked\0settingsClicked\0"
    "exitClicked\0onNewGameButtonClicked\0"
    "onLoadGameButtonClicked\0onSettingsButtonClicked\0"
    "onExitButtonClicked\0onButtonAnimationFinished\0"
    "onBackgroundAnimationUpdate"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Fantasy__MainMenu[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   74,    2, 0x06,    1 /* Public */,
       3,    0,   75,    2, 0x06,    2 /* Public */,
       4,    0,   76,    2, 0x06,    3 /* Public */,
       5,    0,   77,    2, 0x06,    4 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       6,    0,   78,    2, 0x08,    5 /* Private */,
       7,    0,   79,    2, 0x08,    6 /* Private */,
       8,    0,   80,    2, 0x08,    7 /* Private */,
       9,    0,   81,    2, 0x08,    8 /* Private */,
      10,    0,   82,    2, 0x08,    9 /* Private */,
      11,    0,   83,    2, 0x08,   10 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Fantasy::MainMenu::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainMenu *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->newGameClicked(); break;
        case 1: _t->loadGameClicked(); break;
        case 2: _t->settingsClicked(); break;
        case 3: _t->exitClicked(); break;
        case 4: _t->onNewGameButtonClicked(); break;
        case 5: _t->onLoadGameButtonClicked(); break;
        case 6: _t->onSettingsButtonClicked(); break;
        case 7: _t->onExitButtonClicked(); break;
        case 8: _t->onButtonAnimationFinished(); break;
        case 9: _t->onBackgroundAnimationUpdate(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MainMenu::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainMenu::newGameClicked)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MainMenu::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainMenu::loadGameClicked)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (MainMenu::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainMenu::settingsClicked)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (MainMenu::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MainMenu::exitClicked)) {
                *result = 3;
                return;
            }
        }
    }
    (void)_a;
}

const QMetaObject Fantasy::MainMenu::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_Fantasy__MainMenu.offsetsAndSize,
    qt_meta_data_Fantasy__MainMenu,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_Fantasy__MainMenu_t
, QtPrivate::TypeAndForceComplete<MainMenu, std::true_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>
, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>


>,
    nullptr
} };


const QMetaObject *Fantasy::MainMenu::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Fantasy::MainMenu::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Fantasy__MainMenu.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Fantasy::MainMenu::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void Fantasy::MainMenu::newGameClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void Fantasy::MainMenu::loadGameClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void Fantasy::MainMenu::settingsClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void Fantasy::MainMenu::exitClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
