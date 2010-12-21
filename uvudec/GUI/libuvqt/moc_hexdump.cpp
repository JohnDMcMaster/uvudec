/****************************************************************************
** Meta object code from reading C++ file 'hexdump.h'
**
** Created: Tue Dec 21 16:51:03 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "uvqt/hexdump.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'hexdump.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_UVQtHexdump[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_UVQtHexdump[] = {
    "UVQtHexdump\0"
};

const QMetaObject UVQtHexdump::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_UVQtHexdump,
      qt_meta_data_UVQtHexdump, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &UVQtHexdump::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *UVQtHexdump::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *UVQtHexdump::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_UVQtHexdump))
        return static_cast<void*>(const_cast< UVQtHexdump*>(this));
    return QWidget::qt_metacast(_clname);
}

int UVQtHexdump::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_UVQtScrollableHexdump[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_UVQtScrollableHexdump[] = {
    "UVQtScrollableHexdump\0"
};

const QMetaObject UVQtScrollableHexdump::staticMetaObject = {
    { &QAbstractScrollArea::staticMetaObject, qt_meta_stringdata_UVQtScrollableHexdump,
      qt_meta_data_UVQtScrollableHexdump, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &UVQtScrollableHexdump::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *UVQtScrollableHexdump::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *UVQtScrollableHexdump::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_UVQtScrollableHexdump))
        return static_cast<void*>(const_cast< UVQtScrollableHexdump*>(this));
    return QAbstractScrollArea::qt_metacast(_clname);
}

int UVQtScrollableHexdump::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractScrollArea::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
