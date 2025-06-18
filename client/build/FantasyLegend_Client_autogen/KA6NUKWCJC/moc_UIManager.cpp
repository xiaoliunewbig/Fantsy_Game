/****************************************************************************
** Meta object code from reading C++ file 'UIManager.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.3.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../include/ui/managers/UIManager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'UIManager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.3.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Fantasy__UIManager_t {
    const uint offsetsAndSize[76];
    char stringdata0[471];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_Fantasy__UIManager_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_Fantasy__UIManager_t qt_meta_stringdata_Fantasy__UIManager = {
    {
QT_MOC_LITERAL(0, 18), // "Fantasy::UIManager"
QT_MOC_LITERAL(19, 14), // "uiStateChanged"
QT_MOC_LITERAL(34, 0), // ""
QT_MOC_LITERAL(35, 5), // "state"
QT_MOC_LITERAL(41, 22), // "characterStatusUpdated"
QT_MOC_LITERAL(64, 13), // "characterName"
QT_MOC_LITERAL(78, 5), // "stats"
QT_MOC_LITERAL(84, 15), // "skillBarUpdated"
QT_MOC_LITERAL(100, 18), // "QList<QVariantMap>"
QT_MOC_LITERAL(119, 6), // "skills"
QT_MOC_LITERAL(126, 16), // "inventoryUpdated"
QT_MOC_LITERAL(143, 5), // "items"
QT_MOC_LITERAL(149, 13), // "dialogueShown"
QT_MOC_LITERAL(163, 7), // "speaker"
QT_MOC_LITERAL(171, 7), // "content"
QT_MOC_LITERAL(179, 14), // "dialogueHidden"
QT_MOC_LITERAL(194, 14), // "choiceSelected"
QT_MOC_LITERAL(209, 11), // "choiceIndex"
QT_MOC_LITERAL(221, 15), // "battleUIUpdated"
QT_MOC_LITERAL(237, 11), // "playerStats"
QT_MOC_LITERAL(249, 10), // "enemyStats"
QT_MOC_LITERAL(260, 14), // "minimapUpdated"
QT_MOC_LITERAL(275, 7), // "mapData"
QT_MOC_LITERAL(283, 15), // "questLogUpdated"
QT_MOC_LITERAL(299, 6), // "quests"
QT_MOC_LITERAL(306, 17), // "notificationShown"
QT_MOC_LITERAL(324, 7), // "message"
QT_MOC_LITERAL(332, 4), // "type"
QT_MOC_LITERAL(337, 22), // "loadingProgressUpdated"
QT_MOC_LITERAL(360, 8), // "progress"
QT_MOC_LITERAL(369, 12), // "themeChanged"
QT_MOC_LITERAL(382, 5), // "theme"
QT_MOC_LITERAL(388, 15), // "languageChanged"
QT_MOC_LITERAL(404, 8), // "language"
QT_MOC_LITERAL(413, 8), // "updateUI"
QT_MOC_LITERAL(422, 9), // "deltaTime"
QT_MOC_LITERAL(432, 21), // "onNotificationTimeout"
QT_MOC_LITERAL(454, 16) // "onTooltipTimeout"

    },
    "Fantasy::UIManager\0uiStateChanged\0\0"
    "state\0characterStatusUpdated\0characterName\0"
    "stats\0skillBarUpdated\0QList<QVariantMap>\0"
    "skills\0inventoryUpdated\0items\0"
    "dialogueShown\0speaker\0content\0"
    "dialogueHidden\0choiceSelected\0choiceIndex\0"
    "battleUIUpdated\0playerStats\0enemyStats\0"
    "minimapUpdated\0mapData\0questLogUpdated\0"
    "quests\0notificationShown\0message\0type\0"
    "loadingProgressUpdated\0progress\0"
    "themeChanged\0theme\0languageChanged\0"
    "language\0updateUI\0deltaTime\0"
    "onNotificationTimeout\0onTooltipTimeout"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Fantasy__UIManager[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      14,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,  116,    2, 0x06,    1 /* Public */,
       4,    2,  119,    2, 0x06,    3 /* Public */,
       7,    1,  124,    2, 0x06,    6 /* Public */,
      10,    1,  127,    2, 0x06,    8 /* Public */,
      12,    2,  130,    2, 0x06,   10 /* Public */,
      15,    0,  135,    2, 0x06,   13 /* Public */,
      16,    1,  136,    2, 0x06,   14 /* Public */,
      18,    2,  139,    2, 0x06,   16 /* Public */,
      21,    1,  144,    2, 0x06,   19 /* Public */,
      23,    1,  147,    2, 0x06,   21 /* Public */,
      25,    2,  150,    2, 0x06,   23 /* Public */,
      28,    1,  155,    2, 0x06,   26 /* Public */,
      30,    1,  158,    2, 0x06,   28 /* Public */,
      32,    1,  161,    2, 0x06,   30 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      34,    1,  164,    2, 0x08,   32 /* Private */,
      36,    0,  167,    2, 0x08,   34 /* Private */,
      37,    0,  168,    2, 0x08,   35 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString, QMetaType::QVariantMap,    5,    6,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, 0x80000000 | 8,   11,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   13,   14,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   17,
    QMetaType::Void, QMetaType::QVariantMap, 0x80000000 | 8,   19,   20,
    QMetaType::Void, QMetaType::QVariantMap,   22,
    QMetaType::Void, 0x80000000 | 8,   24,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   26,   27,
    QMetaType::Void, QMetaType::Int,   29,
    QMetaType::Void, QMetaType::QString,   31,
    QMetaType::Void, QMetaType::QString,   33,

 // slots: parameters
    QMetaType::Void, QMetaType::Float,   35,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Fantasy::UIManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<UIManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->uiStateChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->characterStatusUpdated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QVariantMap>>(_a[2]))); break;
        case 2: _t->skillBarUpdated((*reinterpret_cast< std::add_pointer_t<QList<QVariantMap>>>(_a[1]))); break;
        case 3: _t->inventoryUpdated((*reinterpret_cast< std::add_pointer_t<QList<QVariantMap>>>(_a[1]))); break;
        case 4: _t->dialogueShown((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 5: _t->dialogueHidden(); break;
        case 6: _t->choiceSelected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->battleUIUpdated((*reinterpret_cast< std::add_pointer_t<QVariantMap>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<QVariantMap>>>(_a[2]))); break;
        case 8: _t->minimapUpdated((*reinterpret_cast< std::add_pointer_t<QVariantMap>>(_a[1]))); break;
        case 9: _t->questLogUpdated((*reinterpret_cast< std::add_pointer_t<QList<QVariantMap>>>(_a[1]))); break;
        case 10: _t->notificationShown((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 11: _t->loadingProgressUpdated((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 12: _t->themeChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 13: _t->languageChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 14: _t->updateUI((*reinterpret_cast< std::add_pointer_t<float>>(_a[1]))); break;
        case 15: _t->onNotificationTimeout(); break;
        case 16: _t->onTooltipTimeout(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<QVariantMap> >(); break;
            }
            break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<QVariantMap> >(); break;
            }
            break;
        case 7:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<QVariantMap> >(); break;
            }
            break;
        case 9:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<QVariantMap> >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (UIManager::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UIManager::uiStateChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (UIManager::*)(const QString & , const QVariantMap & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UIManager::characterStatusUpdated)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (UIManager::*)(const QList<QVariantMap> & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UIManager::skillBarUpdated)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (UIManager::*)(const QList<QVariantMap> & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UIManager::inventoryUpdated)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (UIManager::*)(const QString & , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UIManager::dialogueShown)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (UIManager::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UIManager::dialogueHidden)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (UIManager::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UIManager::choiceSelected)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (UIManager::*)(const QVariantMap & , const QList<QVariantMap> & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UIManager::battleUIUpdated)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (UIManager::*)(const QVariantMap & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UIManager::minimapUpdated)) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (UIManager::*)(const QList<QVariantMap> & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UIManager::questLogUpdated)) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (UIManager::*)(const QString & , const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UIManager::notificationShown)) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (UIManager::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UIManager::loadingProgressUpdated)) {
                *result = 11;
                return;
            }
        }
        {
            using _t = void (UIManager::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UIManager::themeChanged)) {
                *result = 12;
                return;
            }
        }
        {
            using _t = void (UIManager::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&UIManager::languageChanged)) {
                *result = 13;
                return;
            }
        }
    }
}

const QMetaObject Fantasy::UIManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_Fantasy__UIManager.offsetsAndSize,
    qt_meta_data_Fantasy__UIManager,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_Fantasy__UIManager_t
, QtPrivate::TypeAndForceComplete<UIManager, std::true_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QString &, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QString &, std::false_type>, QtPrivate::TypeAndForceComplete<const QVariantMap &, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QList<QVariantMap> &, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QList<QVariantMap> &, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QString &, std::false_type>, QtPrivate::TypeAndForceComplete<const QString &, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<int, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QVariantMap &, std::false_type>, QtPrivate::TypeAndForceComplete<const QList<QVariantMap> &, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QVariantMap &, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QList<QVariantMap> &, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QString &, std::false_type>, QtPrivate::TypeAndForceComplete<const QString &, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<int, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QString &, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<float, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>


>,
    nullptr
} };


const QMetaObject *Fantasy::UIManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Fantasy::UIManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Fantasy__UIManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Fantasy::UIManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void Fantasy::UIManager::uiStateChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Fantasy::UIManager::characterStatusUpdated(const QString & _t1, const QVariantMap & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Fantasy::UIManager::skillBarUpdated(const QList<QVariantMap> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Fantasy::UIManager::inventoryUpdated(const QList<QVariantMap> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Fantasy::UIManager::dialogueShown(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void Fantasy::UIManager::dialogueHidden()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void Fantasy::UIManager::choiceSelected(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void Fantasy::UIManager::battleUIUpdated(const QVariantMap & _t1, const QList<QVariantMap> & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void Fantasy::UIManager::minimapUpdated(const QVariantMap & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void Fantasy::UIManager::questLogUpdated(const QList<QVariantMap> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void Fantasy::UIManager::notificationShown(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void Fantasy::UIManager::loadingProgressUpdated(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}

// SIGNAL 12
void Fantasy::UIManager::themeChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 13
void Fantasy::UIManager::languageChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 13, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
