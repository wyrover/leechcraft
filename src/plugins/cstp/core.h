#ifndef CORE_H
#define CORE_H
#include <deque>
#include <list>
#include <QAbstractItemModel>
#include <QStringList>
#include <boost/shared_ptr.hpp>
#include <interfaces/interfaces.h>

class Task;
class QFile;
class HistoryModel;

namespace _Local
{
	struct ObjectFinder;
	struct SpeedAccumulator;
};

class Core : public QAbstractItemModel
{
	Q_OBJECT
	QStringList Headers_;

	friend class _Local::ObjectFinder;
	friend class _Local::SpeedAccumulator;

	struct TaskDescr
	{
		boost::shared_ptr<Task> Task_;
		boost::shared_ptr<QFile> File_;
		QString Comment_;
		bool ErrorFlag_;
		LeechCraft::TaskParameters Parameters_;
		quint32 ID_;
	};
	typedef std::deque<TaskDescr> tasks_t;
	tasks_t ActiveTasks_;
	HistoryModel *HistoryModel_;
	bool SaveScheduled_;

	std::list<quint32> IDPool_;
	
	explicit Core ();
public:
	enum
	{
		HState
		, HURL
		, HProgress
		, HSpeed
		, HRemaining
		, HDownloading
	};

	virtual ~Core ();
	static Core& Instance ();
	void Release ();
	QAbstractItemModel* GetHistoryModel ();

	int AddTask (const QString&, const QString&,
			const QString&, const QString&,
			LeechCraft::TaskParameters = LeechCraft::SaveInHistory);
	void RemoveTask (const QModelIndex&);
	void RemoveFromHistory (const QModelIndex&);
	void Start (const QModelIndex&);
	void StartI (int);		// FIXME
	void Stop (const QModelIndex&);
	void StopI (int);		// FIXME
	void RemoveAll ();
	void StartAll ();
	void StopAll ();
	qint64 GetDone (int) const;
	qint64 GetTotal (int) const;
	bool IsRunning (int) const;
	qint64 GetTotalDownloadSpeed () const;
	bool CouldDownload (const QString&, LeechCraft::TaskParameters);

	virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
	virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
	virtual Qt::ItemFlags flags (const QModelIndex&) const;
	virtual bool hasChildren (const QModelIndex&) const;
	virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
	virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
	virtual QModelIndex parent (const QModelIndex&) const;
	virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
private slots:
	void done (bool); 
	void updateInterface ();
	void writeSettings ();
private:
	void ReadSettings ();
	void ScheduleSave ();
	tasks_t::const_iterator FindTask (QObject*) const;
	tasks_t::iterator FindTask (QObject*);
	tasks_t::iterator Remove (tasks_t::iterator);
	void AddToHistory (tasks_t::const_iterator);
signals:
	void taskFinished (int);
	void taskRemoved (int);
	void taskError (int, IDirectDownload::Error);
	void fileDownloaded (const QString&);
	void downloadFinished (const QString&);
	void error (const QString&);
	void fileExists (bool*);		// TODO handle this in CSTP
};

#endif

