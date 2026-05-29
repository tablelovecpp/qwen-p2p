/****************************************************************************
** Meta object code from reading C++ file 'discovery.hpp'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../include/network/discovery.hpp"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'discovery.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_p2p__Discovery_t {
    uint offsetsAndSizes[28];
    char stringdata0[15];
    char stringdata1[17];
    char stringdata2[1];
    char stringdata3[10];
    char stringdata4[7];
    char stringdata5[9];
    char stringdata6[5];
    char stringdata7[9];
    char stringdata8[15];
    char stringdata9[6];
    char stringdata10[24];
    char stringdata11[20];
    char stringdata12[15];
    char stringdata13[20];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_p2p__Discovery_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_p2p__Discovery_t qt_meta_stringdata_p2p__Discovery = {
    {
        QT_MOC_LITERAL(0, 14),  // "p2p::Discovery"
        QT_MOC_LITERAL(15, 16),  // "peerCountChanged"
        QT_MOC_LITERAL(32, 0),  // ""
        QT_MOC_LITERAL(33, 9),  // "peerFound"
        QT_MOC_LITERAL(43, 6),  // "peerId"
        QT_MOC_LITERAL(50, 8),  // "PeerInfo"
        QT_MOC_LITERAL(59, 4),  // "info"
        QT_MOC_LITERAL(64, 8),  // "peerLost"
        QT_MOC_LITERAL(73, 14),  // "discoveryError"
        QT_MOC_LITERAL(88, 5),  // "error"
        QT_MOC_LITERAL(94, 23),  // "processPendingDatagrams"
        QT_MOC_LITERAL(118, 19),  // "onAnnouncementTimer"
        QT_MOC_LITERAL(138, 14),  // "onCleanupTimer"
        QT_MOC_LITERAL(153, 19)   // "discoveredPeerCount"
    },
    "p2p::Discovery",
    "peerCountChanged",
    "",
    "peerFound",
    "peerId",
    "PeerInfo",
    "info",
    "peerLost",
    "discoveryError",
    "error",
    "processPendingDatagrams",
    "onAnnouncementTimer",
    "onCleanupTimer",
    "discoveredPeerCount"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_p2p__Discovery[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       1,   71, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   56,    2, 0x06,    2 /* Public */,
       3,    2,   57,    2, 0x06,    3 /* Public */,
       7,    1,   62,    2, 0x06,    6 /* Public */,
       8,    1,   65,    2, 0x06,    8 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      10,    0,   68,    2, 0x08,   10 /* Private */,
      11,    0,   69,    2, 0x08,   11 /* Private */,
      12,    0,   70,    2, 0x08,   12 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, 0x80000000 | 5,    4,    6,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void, QMetaType::QString,    9,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
      13, QMetaType::Int, 0x00015001, uint(0), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject p2p::Discovery::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_p2p__Discovery.offsetsAndSizes,
    qt_meta_data_p2p__Discovery,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_p2p__Discovery_t,
        // property 'discoveredPeerCount'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Discovery, std::true_type>,
        // method 'peerCountChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'peerFound'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const PeerInfo &, std::false_type>,
        // method 'peerLost'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'discoveryError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'processPendingDatagrams'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onAnnouncementTimer'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCleanupTimer'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void p2p::Discovery::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Discovery *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->peerCountChanged(); break;
        case 1: _t->peerFound((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<PeerInfo>>(_a[2]))); break;
        case 2: _t->peerLost((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->discoveryError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->processPendingDatagrams(); break;
        case 5: _t->onAnnouncementTimer(); break;
        case 6: _t->onCleanupTimer(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Discovery::*)();
            if (_t _q_method = &Discovery::peerCountChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Discovery::*)(const QString & , const PeerInfo & );
            if (_t _q_method = &Discovery::peerFound; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (Discovery::*)(const QString & );
            if (_t _q_method = &Discovery::peerLost; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (Discovery::*)(const QString & );
            if (_t _q_method = &Discovery::discoveryError; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<Discovery *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = _t->discoveredPeerCount(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *p2p::Discovery::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *p2p::Discovery::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_p2p__Discovery.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int p2p::Discovery::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void p2p::Discovery::peerCountChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void p2p::Discovery::peerFound(const QString & _t1, const PeerInfo & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void p2p::Discovery::peerLost(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void p2p::Discovery::discoveryError(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
