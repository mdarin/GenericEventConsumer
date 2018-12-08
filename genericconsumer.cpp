#include <iostream>

#include <QObject>

#include <types.h>

#include "genericconsumer.h"

#include "variant2param.h"

GenericConsumer::GenericConsumer(QString complex_id) : _complex_id(complex_id)
{
    _uid.type = "Example Consumer"; //тип
    _uid.instance = "0";            //по идее нужно добавлять ещё и инстанс
    QWidget *form = new QWidget;

    _ui.setupUi(form);  //обычно для формы отдельный класс, но для примера сойдёт и так.
    form->show();
    _ui.lineEditComplex->setText(_complex_id);

    connect(_ui.pushButtonSend, SIGNAL(clicked()), SLOT(sendGenericEvent()));
    connect(this, SIGNAL(prnt(QString,QString)),SLOT(printSomething(QString,QString))/*, Qt::QueuedConnection*/);   //QueuedConnection по идее выставляется автоматичски. Если вызывать синхронно (просто функцией), то будут всякие прикольные проблемы и падения.
    //Ещё можно без сигнала использовать вот такую штуку: QMetaObject::invokeMethod(this, "calculate", Qt::QueuedConnection);
}

void GenericConsumer::processGenericReply(const ::icore::ICCEvent& event)
{
    qDebug("Generic reply!");

    //Извлекаем из события список параметров и переводим его в понятный Qt вид.
    QList <NParam> nparams; //NParam это QPair<QString, Qvariant> — именованный параметр.
    for(uint i=0; i<event.event.params.length(); i++)
        nparams.append( NParam( QString(event.event.params[i].name),    //извлекаем имя
                        param2variant(event.event.params[i].val) ) );   //и значение. param2variant это наша функция. в <types.h>

    QString src_complex( event.event.src_complex );
    QString src_imitator( event.event.src.type );

    qDebug("from %s-%s", qPrintable(src_complex),qPrintable(src_imitator));

    foreach(NParam np, nparams) {   //«Обрабатываем» извлеченые параметры
        QString name(np.first);
        QString value(np.second.toString());
        emit(prnt(name, value));    //и отправляем результат на форму.
    }

//      Вот так ↓ напрямую делать нельзя — будет неожиданно падать с удивительными ошибками.

//    ui.plainTextEdit->appendPlainText("Just text\n");
//    ui.plainTextEdit->appendPlainText(
//                QString("From '%1':\n")
//                .arg(src_type)
//                );
//    for(uint i=0; i<event.params.length(); i++) {
//        qDebug("%d",i);
//        ui.plainTextEdit->appendPlainText(
//                    QString("\t '%1'-'%2'")
//                    .arg(QString(event.params[i].name))
//                    .arg(param2variant(event.params[i].val).toString())
//                    );
//    }
}

void GenericConsumer::setIICDispatcher(icore::ICCEventDispatcher_ptr iccdisp)
{
    _icc_event_dispatcher = icore::ICCEventDispatcher::_duplicate(iccdisp);
}

icore::UnitId* GenericConsumer::id()
{
    icore::UnitId* uid_copy = new icore::UnitId(_uid);
    return uid_copy;
}

void GenericConsumer::sendGenericEvent()
{
    QString targ = _ui.lineEditDevice->text();
    QString targc = _ui.lineEditComplex->text();
    QString event = _ui.lineEditEvent->text();
    QString value = _ui.lineEditValue->text();

    qDebug("GenericConsumer::sendGenericEvent");
    if(_icc_event_dispatcher->_is_nil())
        return;
    //Создаём событие и заполняем его
    icore::ICCEvent iccevent;
    iccevent.event.src = _uid;  //источник события
    iccevent.event.src_complex = _complex_id.toUtf8().constData();

    //комплекс-цель. (В случае армов правильно будет посылать только на свою логику, а она уже в свою очередь пусть посылает в другие комплексы(как в жизни). Но на всякий случай, возможность маршрутизации есть.)
    iccevent.trg_complex = targc.toUtf8().constData();

    //Заполняем список целей-имитаторов. Здесь одна цель.
    icore::TargetList targets;
    icore::ImitatorId target = CORBA::string_dup((char*)targ.toUtf8().constData());
    targets.length(1);
    targets[0] = target;
    iccevent.targets = targets;

    //Подготавливаем параметры для отправки. В примере тоже всего один именованный параметр.
    icore::NVParamList params;
    params.length(1);   //не забываем правильно выставлять размер списка.
        icore::NVParam param;
        param.name = event.toUtf8().constData();
        param.val = variant2param(value);
        params[0] = param;
    iccevent.event.params = params;

    try {
        //посылаем событие в диспетчер. Дальше он сам.
        _icc_event_dispatcher->processGenericEvent(iccevent);
    }
    catch(CORBA::SystemException& ex) {
        std::cerr << "Caught CORBA::SystemException: " << ex._name() << std::endl;
    }
    catch(CORBA::Exception& ex) {
        std::cerr << "Caught CORBA::Exception." << ex._name() << std::endl;
    }
    catch(omniORB::fatalException& fe) {
        std::cerr << "Caught omniORB::fatalException:" << std::endl;
        std::cerr << "  file: " << fe.file() << std::endl;
        std::cerr << "  line: " << fe.line() << std::endl;
        std::cerr << "  mesg: " << fe.errmsg() << std::endl;
    }
    catch(...) {
        std::cerr << "Caught unknown exception." << std::endl;
    }
}

void GenericConsumer::printSomething(QString event, QString value)
{
    _ui.plainTextEdit->appendPlainText(QString("'%1' '%2'")
                                       .arg(event)
                                       .arg(value)
                );
}
