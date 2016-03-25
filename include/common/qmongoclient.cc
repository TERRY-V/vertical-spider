#include "qmongoclient.h"

Q_BEGIN_NAMESPACE

QMongoClient::QMongoClient(const char* uri) :
	conn_(NULL),
	uri_(uri)
{
	Q_ASSERT(instance_.initialized(), "QMongoClient: failed to initialize the client driver!");

	cs_ = ConnectionString::parse(uri_, errmsg_);
	Q_ASSERT(cs_.isValid(), "QMongoClient: parsing connection string (%s) error for (%s)...", uri_.c_str(), errmsg_.c_str());

	conn_ = cs_.connect(errmsg_);
	Q_ASSERT(conn_, "QMongoClient: connecting to (%s) error for (%s)...", uri_.c_str(), errmsg_.c_str());
}

QMongoClient::~QMongoClient()
{
	q_delete< mongo::DBClientBase >(conn_);
}

void QMongoClient::setCollection(const char* collection)
{
	collection_ = collection;
}

const char* QMongoClient::getCollection() const
{
	return collection_.c_str();
}

int32_t QMongoClient::selectAll()
{
	try {
		std::auto_ptr<DBClientCursor> cursor = 
			conn_->query(collection_, mongo::BSONObj());
		while(cursor->more())
			Q_INFO("Data: (%s)!", cursor->next().toString().c_str());
	} catch(const mongo::DBException& e) {
		Q_INFO("QMongoClient: database faild for (%s)...", e.toString());
		return MONGO_ERR;
	}
	return MONGO_OK;
}

int32_t QMongoClient::insert(uint64_t key, const char* value)
{
	if(value == NULL)
		return MONGO_ERR;

	try {
		mongo::BSONObj p = BSON(GENOID \
				<< "key" \
				<< q_to_string(key) \
				<< "value" \
				<< value \
				<< "createdAt" \
				<< mongo::Date_t(dt(QDateTime::now().to_string().c_str())));

		conn_->insert(collection_, p);
	} catch(const mongo::DBException& e) {
		Q_INFO("QMongoClient: database faild for (%s)...", e.toString());
		return MONGO_ERR;
	}
	return MONGO_OK;
}

unsigned long long QMongoClient::count()
{
	return conn_->count(collection_);
}

int32_t QMongoClient::dropCollection()
{
	try {
		conn_->dropCollection(collection_);
	} catch(const mongo::DBException& e) {
		Q_INFO("QMongoClient: database faild for (%s)...", e.toString());
		return MONGO_ERR;
	}
	return MONGO_OK;
}

bool QMongoClient::exists(const char* imgid)
{
	try {
		std::auto_ptr<DBClientCursor> cursor =
			conn_->query(collection_, MONGO_QUERY("imgId" << imgid));
		while(cursor->more()) {
			mongo::BSONObj p = cursor->next();
			return true;
		}
	} catch(const mongo::DBException& e) {
		Q_INFO("QMongoClient: database faild for (%s)...", e.toString());
		return false;
	}

	return false;
}

bool QMongoClient::select(const char* imgid, std::string& imgInfo)
{
	try {
		mongo::BSONObj res = conn_->findOne(collection_, BSON("imgId" << imgid));
		if(!res.isEmpty()) {
			imgInfo = res.getStringField("imgInfo");
			return true;
		}
	} catch(const mongo::DBException& e) {
		Q_INFO("QMongoClient: database faild for (%s)...", e.toString());
		return false;
	}

	return false;
}

int32_t QMongoClient::insert(const char* imgid, const char* imgInfo)
{
	if(imgInfo == NULL)
		return MONGO_ERR;

	try {
		mongo::BSONObj p = BSON(GENOID \
				<< "imgId" \
				<< imgid \
				<< "imgInfo" \
				<< imgInfo \
				<< "createdAt" \
				<< mongo::Date_t(dt(QDateTime::now().to_string().c_str())));

		conn_->insert(collection_, p);
	} catch(const mongo::DBException& e) {
		Q_INFO("QMongoClient: database faild for (%s)...", e.toString());
		return MONGO_ERR;
	}
	return MONGO_OK;
}

int32_t QMongoClient::update(const char* imgid, const char* imgInfo)
{
	if(imgInfo == NULL)
		return MONGO_ERR;

	try {
		mongo::BSONObj p = BSON("imgId" \
				<< imgid);

		mongo::BSONObj after = BSON("imgId" \
				<< imgid \
				<< "imgInfo" \
				<< imgInfo \
				<< "createdAt" \
				<< mongo::Date_t(dt(QDateTime::now().to_string().c_str())));

		conn_->update(collection_, p, after, mongo::UpdateOption_Upsert);
	} catch(const mongo::DBException& e) {
		Q_INFO("QMongoClient: database faild for (%s)...", e.toString());
		return MONGO_ERR;
	}
	return MONGO_OK;
}

int32_t QMongoClient::remove(const char* imgid)
{
	try {
		conn_->remove(collection_, BSON("imgid" << imgid));
	} catch(const mongo::DBException& e) {
		Q_INFO("QMongoClient: database faild for (%s)...", e.toString());
		return MONGO_ERR;
	}
	return MONGO_OK;
}

int32_t QMongoClient::createIndex()
{
	try {
		conn_->createIndex(collection_, BSON("imgid" << 1));
	} catch(const mongo::DBException& e) {
		Q_INFO("QMongoClient: database faild for (%s)...", e.toString());
		return MONGO_ERR;
	}
	return MONGO_OK;
}

unsigned long long QMongoClient::dt(const char* time)
{
	ptime epoch = time_from_string("1970-01-01 00:00:00.000");
	ptime now = time_from_string(time);
	time_duration diff = now - epoch;
	return diff.total_milliseconds();
}

Q_END_NAMESPACE

