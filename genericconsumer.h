#ifndef GENERICCONSUMER_H
#define GENERICCONSUMER_H

#include <iic.hh>
#include <QObject>
#include "ui_form.h"

class GenericConsumer : public QObject, public POA_icore::GenericReplyConsumer
{
Q_OBJECT
public:
    GenericConsumer(QString complex_id);
    /**
     * @brief Принимает входящие события
     * @param event Событие типа IICEvent. Имеет вот такую структуру:
     * Inter-Complex Communication
     *  struct ICCEvent {
     *      ComplexId trg_complex;	// to complex
     *      TargetList targets;     // to imitator
     *      IICEvent event;         // iic event
     *  };
     *
     * ICCEvent в свою очередь включает в себя IICEvent:
     *
     * Inter-Imitator Communication
     *  struct IICEvent {
     *      ComplexId src_complex;	// from complex
     *      UnitId src;             // from imitator
     *      NVParamList params;     // list of named params
     *  };
     */
    void processGenericReply(const ::icore::ICCEvent& event);

    /**
     * @brief Возврашает индентификатор
     * @return UnitId
     */
    icore::UnitId* id();

    /**
     * @brief устанавливает ICCEventDispatcher
     * @param iccdisp CORBA-указатель на диспетчер
     */
    void setIICDispatcher(icore::ICCEventDispatcher_ptr iccdisp);

    //С помощью сигналова и слотов осуществляется синхронизация между GUI-потоком и CORBA-потоком.
signals:
    void prnt(QString, QString);

private slots:
    /**
     * @brief Посылает события
     */
    void sendGenericEvent();
    /**
     * @brief Слот для вывода информации на форму
     * @param event — парметр
     * @param value — значение
     */
    void printSomething(QString event, QString value);

private:
    icore::UnitId _uid;
    Ui::Form _ui;
    icore::ICCEventDispatcher_ptr _icc_event_dispatcher;
    QString _complex_id;
};

#endif // GENERICCONSUMER_H
