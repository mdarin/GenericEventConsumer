/* * * * *
 * Название: hello-genericevent - пирём и отправки сообщений 
 *   имитаторам комплекса из сторонних программ
 * Версия: 0.0.1
 * Описание: производит отправку и приём сообщения посредством брокера
 * Программист разработчик: Дарьин М.В.
 * Программист сопровождения:
 * Дата создания: 28.02.2014
 * Дата последнего изменения:
 * Организация: ЗАО "ОКБ "ИКАР"
 * Лицензия: "AS-IS" "NO WARRENTY"
 * Контакты:
 * почта: 
 * адрес: 
 * 
 * 2014
 * 
 * * */
#include <iostream>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include "genericconsumer.h"


const uint IICDISP_BASE_PORT = 20000;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //в примере не хватает выхода из приложения при закрытии окна.
    // так же в этот момент должна происходить разрегистрация в диспетчере.

    // используется имя комплекса и опционально профиль. По-хорошему нужно добавить ещё uid.
    if (argc<2) {
        qDebug( "usage: %s <complex_id> [profile_id]", argv[0]);
        qDebug( "\tfor example: %s 15ktest A", argv[0]);
        return -1;
    }

    QString complex_id(argv[1]);

    //подготовка аргументов взята из logic
    QList <QByteArray> all_args;
    for (int i = 0; i < argc; ++i) {
            all_args.append(argv[i]);
    }

    bool profile_enabled(false);
    QString profile_id;
    if(argc > 2 && (QString("-noprofile")!=argv[2])) {  //проверяем наличие профиля
        profile_enabled = true;
        profile_id = argv[2];
    }

    //аргументы для инициализации CORBA, с профилем и без
    if(profile_enabled) {

        QFile profile_file("../icfg/profile/"+profile_id+".prfl");
        if (!profile_file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug("ERROR: There are no file named %s", qPrintable(profile_file.fileName()));
            return -1;  //
        }

        typedef QPair <QString, int> IpPort;
        QMap <QString, IpPort >profile;
        QTextStream in_stream(&profile_file);
        while (!in_stream.atEnd()) {
            QString line = in_stream.readLine();
            QStringList qsl = line.split(":");
            if(qsl.size() != 3) {
                qDebug("ERROR: %s is not a profile.", qPrintable(profile_file.fileName()) );
                return -1;  //
            }
            bool ok = false;
            IpPort pp(qsl.at(1), qsl.at(2).toInt(&ok));   //0 - name 1 - IP 2 - port
            profile.insert(qsl.at(0), pp);
            if(!ok){
                qDebug("ERROR: couldn't parse profile %s near %s:%s:%s \n \t %s is not integer",
                       qPrintable(profile_file.fileName()),
                       qPrintable(qsl.at(0)),
                       qPrintable(qsl.at(1)),
                       qPrintable(qsl.at(2)), qPrintable(qsl.at(1))  );
                return -1;  //
            }
        }
        //Append new args to existing
        QMapIterator<QString, IpPort> iter(profile);
        while (iter.hasNext()) {
            iter.next();
            //first append refs to every IIC
            QString initref = QString("ICCEventDispatcher%1=corbaloc::%2:%3/ICCEventDispatcher")
                    .arg(iter.key())
                    .arg(iter.value().first)
                    .arg(IICDISP_BASE_PORT + iter.value().second);
            all_args.append("-ORBInitRef");
            all_args.append(initref.toLatin1());
        }
    } else {    //profile disabled. Default object references
        QString initref = QString("ICCEventDispatcher%1=corbaloc::localhost:%2/ICCEventDispatcher")
                .arg(complex_id)
                .arg(IICDISP_BASE_PORT);
        all_args.append("-ORBInitRef");
        all_args.append(initref.toLatin1());
    }

    // создаем наш объект
    GenericConsumer * consumer = new GenericConsumer(complex_id);

    //извлекаем аргументы
    qDebug("CORBA will be initialized with following args:");
    int cargc = all_args.count();
    char **cargv = new char*[cargc];
    for (int i = 0; i < cargc; ++i) {
        std::cout << all_args[i].data();
        cargv[i] = all_args[i].data();
    if (i % 2)
        std::cout << (i > 2 ? "\"\n\t" : " ");
    else
        std::cout << (i > 3 ? "=\"" : " ");
    }
    std::cout << std::endl;

    //инициализируем CORBA
    CORBA::ORB_var orb = CORBA::ORB_init(cargc, cargv);
    CORBA::Object_var obj;

    try {
        obj = orb->resolve_initial_references("RootPOA");
        PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);
        PortableServer::ObjectId_var consumer_var = poa->activate_object(consumer); //активируем консьюмер
        PortableServer::POAManager_var pman = poa->the_POAManager();
        pman->activate();

        //находим диспетчер
        QString icc_initref = QString("ICCEventDispatcher") + complex_id;
        qDebug("ICC INIT REF: *%s*", icc_initref.toAscii().constData());
        CORBA::Object_var obj = orb->resolve_initial_references(icc_initref.toAscii().constData());
        icore::ICCEventDispatcher_var icc_event_dispatcher =
                icore::ICCEventDispatcher::_narrow(obj);
        consumer->setIICDispatcher(icc_event_dispatcher);   //передаем диспетчер в наш объект
        icc_event_dispatcher->registerGenericReplyConsumer(consumer->_this());  //регистрируемся в диспетчере
    }
    catch (CORBA::NO_RESOURCES&) {
        std::cerr << "Caught CORBA::NO_RESOURCES exception.\n\t You must configure omniORB "
                << "with the location of the naming service." << std::endl;
    }
    catch(CORBA::SystemException& ex) {
        std::cerr << "Caught CORBA::SystemException: " << ex._name() << std::endl;
    }
    catch(CORBA::Exception& ex) {
        std::cerr << "Caught CORBA::Exception: " << ex._name() << std::endl;
    }
    catch(omniORB::fatalException& fe) {
        std::cerr << "Caught omniORB::fatalException:" << std::endl;
        std::cerr << " file: " << fe.file() << std::endl;
        std::cerr << " line: " << fe.line() << std::endl;
        std::cerr << " mesg: " << fe.errmsg() << std::endl;
    }
    catch(...) {
        std::cerr << "Caught unknown exception." << std::endl;
    }

    return a.exec();
}
