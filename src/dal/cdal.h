/*
 * cdal.h
 *
 *  Created on: Jul 6, 2018
 *      Author: konstantinnosach
 */

#ifndef DAL_CDAL_H_
#define DAL_CDAL_H_
#include <libgda/libgda.h>
#include "../account.h"
#include "../mqtt/packets/publish.h"

GdaConnection *open_db_connection (void);

void save_changes (struct Account * account);
void save_topic_to_db (const char * topic_name, int qos);
void remove_topic_from_db(const char * topic_name);
void save_message (const char * content, const char * topic_name, int qos, int retain, int dup, int is_incoming);
struct MqttModel * get_accounts();
struct MqttModel * get_topics_from_db();
struct MqttModel * get_messages_from_db();

typedef void (*SaveAccount)(struct Account * account);
typedef void (*SaveTopic)(struct Topic * topic);
typedef void (*SaveMessage)(const char * content, const char * topic_name, int qos, int retain, int dup, int is_incoming);

typedef struct MqttModel * (*GetAccounts)(void);

void create_account_table_if_not_exist (GdaConnection *cnc);
void insert_data (GdaConnection *cnc, struct Account * account);
void set_account_default(int id);
void remove_account_from_db(int id);



struct MqttModel {
	struct Account * account;
	int account_size;
	struct CommonTopic * topic;
	int topic_size;
	struct CommonPublish * common_publish;
	int publish_size;

	SaveAccount  save_acc_pt;
	SaveTopic  save_topic_pt;
	SaveMessage  save_message_pt;
	GetAccounts get_accounts_pt;
};




#endif /* DAL_CDAL_H_ */
