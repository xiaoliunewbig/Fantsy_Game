/****************************************************************************
** Meta object code from reading C++ file 'GameScene.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.3.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../include/ui/scenes/GameScene.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GameScene.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.3.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Fantasy__GameScene_t {
    const uint offsetsAndSize[30];
    char stringdata0[179];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_Fantasy__GameScene_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_Fantasy__GameScene_t qt_meta_stringdata_Fantasy__GameScene = {
    {
QT_MOC_LITERAL(0, 18), // "Fantasy::GameScene"
QT_MOC_LITERAL(19, 14), // "characterMoved"
QT_MOC_LITERAL(34, 0), // ""
QT_MOC_LITERAL(35, 10), // "Character*"
QT_MOC_LITERAL(46, 9), // "character"
QT_MOC_LITERAL(56, 8), // "position"
QT_MOC_LITERAL(65, 17), // "characterSelected"
QT_MOC_LITERAL(83, 12), // "sceneClicked"
QT_MOC_LITERAL(96, 10), // "keyPressed"
QT_MOC_LITERAL(107, 3), // "key"
QT_MOC_LITERAL(111, 11), // "keyReleased"
QT_MOC_LITERAL(123, 6), // "update"
QT_MOC_LITERAL(130, 11), // "updateScene"
QT_MOC_LITERAL(142, 9), // "deltaTime"
QT_MOC_LITERAL(152, 26) // "onCharacterPositionChanged"

    },
    "Fantasy::GameScene\0characterMoved\0\0"
    "Character*\0character\0position\0"
    "characterSelected\0sceneClicked\0"
    "keyPressed\0key\0keyReleased\0update\0"
    "updateScene\0deltaTime\0onCharacterPositionChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Fantasy__GameScene[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   62,    2, 0x06,    1 /* Public */,
       6,    1,   67,    2, 0x06,    4 /* Public */,
       7,    1,   70,    2, 0x06,    6 /* Public */,
       8,    1,   73,    2, 0x06,    8 /* Public */,
      10,    1,   76,    2, 0x06,   10 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      11,    0,   79,    2, 0x08,   12 /* Private */,
      12,    1,   80,    2, 0x08,   13 /* Private */,
      14,    2,   83,    2, 0x08,   15 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QVector2D,    4,    5,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::QVector2D,    5,
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void, QMetaType::Int,    9,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Float,   13,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QVector2D,    4,    5,

       0        // eod
};

void Fantasy::GameScene::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<GameScene *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->characterMoved((*reinterpret_cast< std::add_pointer_t<Character*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QVector2D>>(_a[2]))); break;
        case 1: _t->characterSelected((*reinterpret_cast< std::add_pointer_t<Character*>>(_a[1]))); break;
        case 2: _t->sceneClicked((*reinterpret_cast< std::add_pointer_t<QVector2D>>(_a[1]))); break;
        case 3: _t->keyPressed((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->keyReleased((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->update(); break;
        case 6: _t->updateScene((*reinterpret_cast< std::add_pointer_t<float>>(_a[1]))); break;
        case 7: _t->onCharacterPositionChanged((*reinterpret_cast< std::add_pointer_t<Character*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QVector2D>>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (GameScene::*)(Character * , const QVector2D & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GameScene::characterMoved)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (GameScene::*)(Character * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GameScene::characterSelected)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (GameScene::*)(const QVector2D & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GameScene::sceneClicked)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (GameScene::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GameScene::keyPressed)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (GameScene::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&GameScene::keyReleased)) {
                *result = 4;
                return;
            }
        }
    }
}

const QMetaObject Fantasy::GameScene::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_Fantasy__GameScene.offsetsAndSize,
    qt_meta_data_Fantasy__GameScene,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_Fantasy__GameScene_t
, QtPrivate::TypeAndForceComplete<GameScene, std::true_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<Character *, std::false_type>, QtPrivate::TypeAndForceComplete<const QVector2D &, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<Character *, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QVector2D &, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<int, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<int, std::false_type>
, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<float, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<Character *, std::false_type>, QtPrivate::TypeAndForceComplete<const QVector2D &, std::false_type>


>,
    nullptr
} };


const QMetaObject *Fantasy::GameScene::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Fantasy::GameScene::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Fantasy__GameScene.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Fantasy::GameScene::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void Fantasy::GameScene::characterMoved(Character * _t1, const QVector2D & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Fantasy::GameScene::characterSelected(Character * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Fantasy::GameScene::sceneClicked(const QVector2D & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Fantasy::GameScene::keyPressed(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Fantasy::GameScene::keyReleased(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
