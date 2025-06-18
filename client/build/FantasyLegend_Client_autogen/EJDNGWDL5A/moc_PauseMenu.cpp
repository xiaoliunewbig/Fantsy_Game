/****************************************************************************
** Meta object code from reading C++ file 'PauseMenu.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.3.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../include/ui/components/PauseMenu.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PauseMenu.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.3.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Fantasy__PauseMenu_t {
    const uint offsetsAndSize[24];
    char stringdata0[228];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_Fantasy__PauseMenu_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_Fantasy__PauseMenu_t qt_meta_stringdata_Fantasy__PauseMenu = {
    {
QT_MOC_LITERAL(0, 18), // "Fantasy::PauseMenu"
QT_MOC_LITERAL(19, 13), // "resumeClicked"
QT_MOC_LITERAL(33, 0), // ""
QT_MOC_LITERAL(34, 15), // "saveGameClicked"
QT_MOC_LITERAL(50, 15), // "loadGameClicked"
QT_MOC_LITERAL(66, 15), // "settingsClicked"
QT_MOC_LITERAL(82, 21), // "backToMainMenuClicked"
QT_MOC_LITERAL(104, 21), // "onResumeButtonClicked"
QT_MOC_LITERAL(126, 23), // "onSaveGameButtonClicked"
QT_MOC_LITERAL(150, 23), // "onLoadGameButtonClicked"
QT_MOC_LITERAL(174, 23), // "onSettingsButtonClicked"
QT_MOC_LITERAL(198, 29) // "onBackToMainMenuButtonClicked"

    },
    "Fantasy::PauseMenu\0resumeClicked\0\0"
    "saveGameClicked\0loadGameClicked\0"
    "settingsClicked\0backToMainMenuClicked\0"
    "onResumeButtonClicked\0onSaveGameButtonClicked\0"
    "onLoadGameButtonClicked\0onSettingsButtonClicked\0"
    "onBackToMainMenuButtonClicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Fantasy__PauseMenu[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   74,    2, 0x06,    1 /* Public */,
       3,    0,   75,    2, 0x06,    2 /* Public */,
       4,    0,   76,    2, 0x06,    3 /* Public */,
       5,    0,   77,    2, 0x06,    4 /* Public */,
       6,    0,   78,    2, 0x06,    5 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
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
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Fantasy::PauseMenu::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PauseMenu *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->resumeClicked(); break;
        case 1: _t->saveGameClicked(); break;
        case 2: _t->loadGameClicked(); break;
        case 3: _t->settingsClicked(); break;
        case 4: _t->backToMainMenuClicked(); break;
        case 5: _t->onResumeButtonClicked(); break;
        case 6: _t->onSaveGameButtonClicked(); break;
        case 7: _t->onLoadGameButtonClicked(); break;
        case 8: _t->onSettingsButtonClicked(); break;
        case 9: _t->onBackToMainMenuButtonClicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PauseMenu::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PauseMenu::resumeClicked)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PauseMenu::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PauseMenu::saveGameClicked)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (PauseMenu::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PauseMenu::loadGameClicked)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (PauseMenu::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PauseMenu::settingsClicked)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (PauseMenu::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PauseMenu::backToMainMenuClicked)) {
                *result = 4;
                return;
            }
        }
    }
    (void)_a;
}

const QMetaObject Fantasy::PauseMenu::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_Fantasy__PauseMenu.offsetsAndSize,
    qt_meta_data_Fantasy__PauseMenu,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_Fantasy__PauseMenu_t
, QtPrivate::TypeAndForceComplete<PauseMenu, std::true_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>
, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>


>,
    nullptr
} };


const QMetaObject *Fantasy::PauseMenu::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Fantasy::PauseMenu::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Fantasy__PauseMenu.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Fantasy::PauseMenu::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void Fantasy::PauseMenu::resumeClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void Fantasy::PauseMenu::saveGameClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void Fantasy::PauseMenu::loadGameClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void Fantasy::PauseMenu::settingsClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void Fantasy::PauseMenu::backToMainMenuClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
