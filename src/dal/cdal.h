/**
* Mobius Software LTD
* Copyright 2015-2018, Mobius Software LTD
*
* This is free software; you can redistribute it and/or modify it
* under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 2.1 of
* the License, or (at your option) any later version.
*
* This software is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this software; if not, write to the Free
* Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301 USA, or see the FSF site: http://www.fsf.org.
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
void remove_account_messages_from_db(int id);
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
void remove_account_topics_from_db(int id);
gboolean is_account_with_client_id_exist (char * client_id);
gboolean is_topic_exist (int id, char * topic_name);



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
