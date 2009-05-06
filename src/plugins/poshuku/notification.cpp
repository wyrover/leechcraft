#include "notification.h"
#include <stdexcept>
#include <QVBoxLayout>

Notification::Notification (QWidget *parent)
: QWidget (parent)
{
	QVBoxLayout *lay = qobject_cast<QVBoxLayout*> (parent->layout ());
	if (!lay)
		throw std::runtime_error ("Passed parent object has no QVBoxLayout");
	lay->addWidget (this);
}

