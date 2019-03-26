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

#include <libgda/libgda.h>
#include <sql-parser/gda-sql-parser.h>
#include <gtk/gtk.h>
#include <string.h>
#include "../account.h"
#include "../common_topic.h"
#include "../common_publish.h"
#include "cdal.h"

GdaConnection *open_db_connection (void);
void run_sql_non_select (GdaConnection *cnc, const gchar *sql);

GdaConnection *cnc;

/*
 * Open a connection to the *.db file
 */
GdaConnection * open_db_connection ()
{
    GdaConnection *cnc;
    GError *error = NULL;
	GdaSqlParser *parser;

	/* open connection */
    cnc = gda_connection_open_from_string ( "SQLite", "DB_DIR=.;DB_NAME=cclient_db_name",
			NULL, GDA_CONNECTION_OPTIONS_NONE, &error);
    if (!cnc) {
    	g_print ("Could not open connection to SQLite database in example_db.db file: %s\n",
    			error && error->message ? error->message : "No detail");
    	exit (1);
    }
	/* create an SQL parser */
	parser = gda_connection_create_parser (cnc);
	if (!parser) /* @cnc does not provide its own parser => use default one */
		parser = gda_sql_parser_new ();
	/* attach the parser object to the connection */
	g_object_set_data_full (G_OBJECT (cnc), "parser", parser, g_object_unref);

    return cnc;
}

static gint get_default_account_id () {

	GdaDataModel *data_model;
	GdaSqlParser *parser;
	GdaStatement *stmt;
	GError *error = NULL;

	gchar *sql = "SELECT account_id FROM account WHERE is_default=1";
	parser = g_object_get_data (G_OBJECT (cnc), "parser");
	stmt = gda_sql_parser_parse_string (parser, sql, NULL, NULL);
	data_model = gda_connection_statement_execute_select (cnc, stmt, NULL, &error);
	g_object_unref (stmt);
		if (!data_model)
			g_error ("Could not get the default account of the 'account' table: %s\n",
	                         error && error->message ? error->message : "No detail");

	gint account_id_int = g_value_get_int(gda_data_model_get_value_at(data_model, 0, 0, NULL));
	return account_id_int;
}

gboolean is_account_with_client_id_exist (char * client_id) {

	GdaDataModel *data_model;
	GdaSqlParser *parser;
	GdaStatement *stmt;
	GError *error = NULL;

	gchar sql [256] = {};
	strcat(sql, "SELECT COUNT(*) FROM account WHERE client_id=");
	strcat(sql, client_id);
	parser = g_object_get_data (G_OBJECT (cnc), "parser");
	stmt = gda_sql_parser_parse_string (parser, sql, NULL, NULL);
	data_model = gda_connection_statement_execute_select (cnc, stmt, NULL, &error);
	g_object_unref (stmt);
		if (!data_model)
			g_error ("Could not get the default account of the 'account' table: %s\n",
	                         error && error->message ? error->message : "No detail");

	gint row_number = g_value_get_int(gda_data_model_get_value_at(data_model, 0, 0, NULL));
	if(row_number == 0)
		return FALSE;
	else
		return TRUE;
}

void create_account_table_if_not_exist (GdaConnection *cnc)
{
    run_sql_non_select(cnc, "CREATE table IF NOT EXISTS account ("
    		"account_id integer not null primary key AUTOINCREMENT,"
            "protocol integer not null,"
    		"username text,"
    		"password text,"
    		"client_id text,"
    		"server_host text not null,"
    		"server_port integer not null,"
    		"clean_session integer,"
    		"keep_alive integer,"
    		"will text,"
    		"will_topic text,"
    		"is_retain integer,"
    		"qos integer,"
    		"is_secure integer,"
    		"certificate text,"
    		"certificate_password text,"
            "is_default integer not null)");

}

void create_topic_table_if_not_exist (GdaConnection *cnc)
{
    run_sql_non_select(cnc, "CREATE table IF NOT EXISTS topic ("
    		"topic_id integer not null primary key AUTOINCREMENT,"
    		"topic_name text not null,"
    		"qos integer not null,"
    		"topic_account INTEGER,"
    		"FOREIGN KEY(topic_account) REFERENCES account(account_id))");
}

void create_message_table_if_not_exist (GdaConnection *cnc)
{
    run_sql_non_select(cnc, "CREATE table IF NOT EXISTS message ("
    		"message_id integer not null primary key AUTOINCREMENT,"
    		"topic_name text not null,"
    		"content text not null,"
    		"qos integer not null,"
    		"is_incoming integer not null,"
    		"is_retain integer not null,"
    		"is_dub integer not null,"
    		"message_account INTEGER,"
    		"FOREIGN KEY(message_account) REFERENCES account(account_id))");
}

void insert_data (GdaConnection *cnc, struct Account * account)
{
	gboolean res;
	GError *error = NULL;
	GValue *protocol,*server_host,*server_port,
	*clean_session, * keep_alive, *is_retain, *qos, *is_default;
	GValue *username = NULL;
	GValue *password = NULL;
	GValue *will = NULL;
	GValue *will_topic = NULL;
	GValue *client_id = NULL;
	GValue *is_secure = NULL;
	GValue *certificate = NULL;
	GValue *certificate_password = NULL;



	protocol = gda_value_new (G_TYPE_INT);
	g_value_set_int (protocol, account->protocol);

	if(account->protocol == MQTT || account->protocol == WEBSOCKETS || account->protocol == AMQP) {
		username = gda_value_new_from_string (account->username, G_TYPE_STRING);
		password = gda_value_new_from_string (account->password, G_TYPE_STRING);
	}
	
	client_id = gda_value_new_from_string (account->client_id, G_TYPE_STRING);

	server_host = gda_value_new_from_string (account->server_host, G_TYPE_STRING);

	server_port = gda_value_new (G_TYPE_INT);
	g_value_set_int (server_port, account->server_port);
	clean_session = gda_value_new (G_TYPE_INT);
	g_value_set_int (clean_session, account->clean_session);
	keep_alive = gda_value_new (G_TYPE_INT);
	g_value_set_int (keep_alive, account->keep_alive);

	if((account->protocol == MQTT || account->protocol == MQTT_SN) &&  account->will != NULL && account->will_topic != NULL) {
		will = gda_value_new_from_string (account->will, G_TYPE_STRING);
		will_topic = gda_value_new_from_string (account->will_topic, G_TYPE_STRING);
	}
	is_retain = gda_value_new (G_TYPE_INT);
	g_value_set_int (is_retain, account->is_retain);
	qos = gda_value_new (G_TYPE_INT);
	g_value_set_int (qos, account->qos);

	is_secure = gda_value_new (G_TYPE_INT);
	g_value_set_int (is_secure, account->is_secure);

	if(account->certificate != NULL)
		certificate = gda_value_new_from_string (account->certificate, G_TYPE_STRING);
	if(account->certificate_password != NULL)
		certificate_password = gda_value_new_from_string (account->certificate_password, G_TYPE_STRING);

	is_default = gda_value_new (G_TYPE_INT);
	g_value_set_int (is_default, account->is_default);
	res = gda_connection_insert_row_into_table (cnc, "account", &error,
			"protocol", protocol,
			"username", username,
			"password", password,
			"client_id", client_id,
			"server_host", server_host,
			"server_port", server_port,
			"clean_session", clean_session,
			"keep_alive", keep_alive,
			"will", will,
			"will_topic", will_topic,
			"is_retain", is_retain,
			"qos", qos,
			"is_secure", is_secure,
			"certificate", certificate,
			"certificate_password", certificate_password,
			"is_default", is_default,
			NULL);

	if (!res) {
		g_error ("Could not INSERT data into the 'accounts' table: %s\n",
			 error && error->message ? error->message : "No detail");
	}

	gda_value_free (protocol);
	gda_value_free (username);
	gda_value_free (password);
	gda_value_free (client_id);
	gda_value_free (server_host);
	gda_value_free (server_port);
	gda_value_free (clean_session);
	gda_value_free (keep_alive);
	gda_value_free (will);
	gda_value_free (will_topic);
	gda_value_free (is_retain);
	gda_value_free (qos);
	gda_value_free (is_secure);
	gda_value_free (certificate);
	gda_value_free (certificate_password);
	gda_value_free (is_default);

}

void insert_topic_data (GdaConnection *cnc, const char * _topic_name, int _qos)
{
	gboolean res;
	GdaDataModel *data_model;
	GdaSqlParser *parser;
	GdaStatement *stmt;
	GError *error = NULL;

	gchar *sql = "SELECT account_id FROM account WHERE is_default=1";
	parser = g_object_get_data (G_OBJECT (cnc), "parser");
	stmt = gda_sql_parser_parse_string (parser, sql, NULL, NULL);
	data_model = gda_connection_statement_execute_select (cnc, stmt, NULL, &error);
	g_object_unref (stmt);
		if (!data_model)
			g_error ("Could not get the default account of the 'account' table: %s\n",
	                         error && error->message ? error->message : "No detail");

	//gda_data_model_dump (data_model, stdout);
	gint account_id_int = g_value_get_int(gda_data_model_get_value_at(data_model, 0, 0, NULL));

	GValue *topic_name, *qos, *account_id;

	topic_name = gda_value_new_from_string (_topic_name, G_TYPE_STRING);
	qos = gda_value_new (G_TYPE_INT);
	g_value_set_int (qos, _qos);
	account_id = gda_value_new (G_TYPE_INT);
	g_value_set_int (account_id, account_id_int);

	res = gda_connection_insert_row_into_table (cnc, "topic", &error,
			"topic_name", topic_name,
			"qos", qos,
			"topic_account", account_id,
			NULL);

	if (!res) {
		g_error ("Could not INSERT data into the 'topics' table: %s\n",
			 error && error->message ? error->message : "No detail");
	}

	gda_value_free (topic_name);
	gda_value_free (qos);
	gda_value_free (account_id);
}

static void remove_account(int id) {

	gchar str1 [256] = {};
	gchar str2 [256] = {};
	gchar str3 [256] = {};
	char account_id_string [3];
	sprintf(account_id_string, "%d", id);

	//remove messages
	strcat(str1, "DELETE FROM message WHERE message_account=");
	strcat(str1, account_id_string);
	run_sql_non_select(cnc, str1);

	//remove topics
	strcat(str2, "DELETE FROM topic WHERE topic_account=");
	strcat(str2, account_id_string);
	run_sql_non_select(cnc, str2);

	strcat(str3, "DELETE FROM account WHERE account_id=");
	strcat(str3, account_id_string);
	run_sql_non_select(cnc, str3);

}

void remove_account_messages_from_db(int id) {
	gchar str1 [256] = {};
	char account_id_string [3];
	sprintf(account_id_string, "%d", id);
	//remove messages
	strcat(str1, "DELETE FROM message WHERE message_account=");
	strcat(str1, account_id_string);
	run_sql_non_select(cnc, str1);
}

void remove_account_topics_from_db(int id) {
	gchar str1 [256] = {};
	char account_id_string [3];
	sprintf(account_id_string, "%d", id);
	//remove topics
	strcat(str1, "DELETE FROM topic WHERE topic_account=");
	strcat(str1, account_id_string);
	run_sql_non_select(cnc, str1);
}


static void remove_topic (GdaConnection *cnc, const char * topic_name)
{

	GdaDataModel *data_model;
	GdaSqlParser *parser;
	GdaStatement *stmt;
	GError *error = NULL;

	gchar *sql = "SELECT account_id FROM account WHERE is_default=1";
	parser = g_object_get_data (G_OBJECT (cnc), "parser");
	stmt = gda_sql_parser_parse_string (parser, sql, NULL, NULL);
	data_model = gda_connection_statement_execute_select (cnc, stmt, NULL, &error);
	g_object_unref (stmt);
		if (!data_model)
			g_error ("Could not get the default account of the 'account' table: %s\n",
	                         error && error->message ? error->message : "No detail");

	//gda_data_model_dump (data_model, stdout);
	gint account_id_int = g_value_get_int(gda_data_model_get_value_at(data_model, 0, 0, NULL));

	gchar str [256] = {};
	strcat(str, "DELETE FROM topic WHERE topic_account=");
	char account_id_string [3];
	sprintf(account_id_string, "%d", account_id_int);
	strcat(str, account_id_string);
	strcat(str, " and topic_name='");
	strcat(str, topic_name);
	strcat(str, "'");
	run_sql_non_select(cnc, str);

}

void insert_message_data (GdaConnection *cnc, const char * _content, const char * _topic_name, int _qos, int _retain, int _dup, int _is_incoming)
{
	gboolean res;
	GdaDataModel *data_model;
	GdaSqlParser *parser;
	GdaStatement *stmt;
	GError *error = NULL;

	gchar *sql = "SELECT account_id FROM account WHERE is_default=1";
	parser = g_object_get_data (G_OBJECT (cnc), "parser");
	stmt = gda_sql_parser_parse_string (parser, sql, NULL, NULL);
	data_model = gda_connection_statement_execute_select (cnc, stmt, NULL, &error);
	g_object_unref (stmt);
		if (!data_model)
			g_error ("Could not get the default account of the 'account' table: %s\n",
	                         error && error->message ? error->message : "No detail");

	//gda_data_model_dump (data_model, stdout);
	gint account_id_int = g_value_get_int(gda_data_model_get_value_at(data_model, 0, 0, NULL));

	GValue *topic_name, *content, *qos, *is_incoming, *is_retain, *is_dub, *account_id;

	topic_name = gda_value_new_from_string (_topic_name, G_TYPE_STRING);
	content = gda_value_new_from_string (_content, G_TYPE_STRING);
	qos = gda_value_new (G_TYPE_INT);
	g_value_set_int (qos, _qos);
	is_retain = gda_value_new (G_TYPE_INT);
	g_value_set_int (is_retain, _retain);
	is_dub = gda_value_new (G_TYPE_INT);
	g_value_set_int (is_dub, _dup);
	is_incoming = gda_value_new (G_TYPE_INT);
	g_value_set_int (is_incoming, _is_incoming);
	account_id = gda_value_new (G_TYPE_INT);
	g_value_set_int (account_id, account_id_int);

	res = gda_connection_insert_row_into_table (cnc, "message", &error,
			"topic_name", topic_name,
			"content", content,
			"qos", qos,
			"is_retain", is_retain,
			"is_dub", is_dub,
			"is_incoming", is_incoming,
			"message_account", account_id,
			NULL);

	if (!res) {
		g_error ("Could not INSERT data into the 'message' table: %s\n",
			 error && error->message ? error->message : "No detail");
	}

	gda_value_free (topic_name);
	gda_value_free (content);
	gda_value_free (qos);
	gda_value_free (is_retain);
	gda_value_free (is_dub);
	gda_value_free (is_incoming);
	gda_value_free (account_id);
}


void set_default (int id)
{
	char str [256] = {};
	strcat(str, "UPDATE account SET is_default=1 WHERE account_id=");
	char id_string [5];
	sprintf(id_string, "%d", id);
	strcat(str, id_string);

	gchar *sql = str;
	run_sql_non_select(cnc,sql);
}

static void set_default_all ()
{
	gchar *sql = "UPDATE account SET is_default=0";
	run_sql_non_select(cnc,sql);
}



void run_sql_non_select (GdaConnection *cnc, const gchar *sql)
{
    GdaStatement *stmt;
    GError *error = NULL;
    gint nrows;
	const gchar *remain;
	GdaSqlParser *parser;
	//printf("SQL: %s \n",sql);

	parser = g_object_get_data (G_OBJECT (cnc), "parser");
	stmt = gda_sql_parser_parse_string (parser, sql, &remain, &error);
//	if (remain)
//		g_print ("REMAINS: %s\n", remain);

    nrows = gda_connection_statement_execute_non_select (cnc, stmt, NULL, NULL, &error);
    if (nrows == -1)
    	g_error ("NON SELECT error: %s\n", error && error->message ? error->message : "no detail");
	g_object_unref (stmt);
}

struct MqttModel * get_accounts_from_db (GdaConnection *cnc) {

	GdaDataModel *data_model;
	GdaSqlParser *parser;
	GdaStatement *stmt;
	gchar *sql = "SELECT "
			"account_id,"
			"protocol,"
			"username,"
			"password,"
			"client_id,"
			"server_host,"
			"server_port,"
			"clean_session,"
			"keep_alive,"
			"will,"
			"will_topic,"
			"is_retain,"
			"qos,"
			"is_secure,"
			"certificate,"
			"certificate_password,"
			"is_default"
			" FROM account";
	GError *error = NULL;

	parser = g_object_get_data (G_OBJECT (cnc), "parser");
	stmt = gda_sql_parser_parse_string (parser, sql, NULL, NULL);
	data_model = gda_connection_statement_execute_select (cnc, stmt, NULL, &error);
	g_object_unref (stmt);
        if (!data_model)
                g_error ("Could not get the contents of the 'account' table: %s\n",
                         error && error->message ? error->message : "No detail");
    gint rows = gda_data_model_get_n_rows(data_model);
    //gda_data_model_dump (data_model, stdout);
    if(rows < 1) {
    	g_object_unref (data_model);
    	return NULL;
    } else {
    	struct Account * account = malloc (sizeof (struct Account) * rows);
    	for(int i = 0; i < rows; i++) {
    		account[i].id = g_value_get_int(gda_data_model_get_value_at(data_model, 0, i, NULL));
    		account[i].protocol = g_value_get_int(gda_data_model_get_value_at(data_model, 1, i, NULL));

    		if(gda_data_model_get_value_at(data_model, 2, i, NULL)->data->v_pointer != NULL)
    			account[i].username = g_value_get_string(gda_data_model_get_value_at(data_model, 2, i, NULL));
    		else
    			account[i].username = NULL;

    		if(gda_data_model_get_value_at(data_model, 3, i, NULL)->data->v_pointer != NULL)
    			account[i].password = g_value_get_string(gda_data_model_get_value_at(data_model, 3, i, NULL));
    		else
    			account[i].password = NULL;

    		if(gda_data_model_get_value_at(data_model, 4, i, NULL)->data->v_pointer != NULL)
    			account[i].client_id = g_value_get_string(gda_data_model_get_value_at(data_model, 4, i, NULL));
    		else
    			account[i].client_id = NULL;

    		account[i].server_host = g_value_get_string(gda_data_model_get_value_at(data_model, 5, i, NULL));
    		account[i].server_port = g_value_get_int(gda_data_model_get_value_at(data_model, 6, i, NULL));
    		account[i].clean_session = g_value_get_int(gda_data_model_get_value_at(data_model, 7, i, NULL));
    		account[i].keep_alive = g_value_get_int(gda_data_model_get_value_at(data_model, 8, i, NULL));

    		if(gda_data_model_get_value_at(data_model, 9, i, NULL)->data->v_pointer != NULL)
    			account[i].will = g_value_get_string(gda_data_model_get_value_at(data_model, 9, i, NULL));
    		else
    			account[i].will = NULL;

    		if(gda_data_model_get_value_at(data_model, 10, i, NULL)->data->v_pointer != NULL)
    			account[i].will_topic = g_value_get_string(gda_data_model_get_value_at(data_model, 10, i, NULL));
    		else
    			account[i].will_topic = NULL;

    		account[i].is_retain = g_value_get_int(gda_data_model_get_value_at(data_model, 11, i, NULL));
    		account[i].qos = g_value_get_int(gda_data_model_get_value_at(data_model, 12, i, NULL));
    		//ssl
    		account[i].is_secure = g_value_get_int(gda_data_model_get_value_at(data_model, 13, i, NULL));

    		if(gda_data_model_get_value_at(data_model, 14, i, NULL)->data->v_pointer != NULL)
    			account[i].certificate = g_value_get_string(gda_data_model_get_value_at(data_model, 14, i, NULL));
    		else
    			account[i].certificate = NULL;

    		if(gda_data_model_get_value_at(data_model, 15, i, NULL)->data->v_pointer != NULL)
    			account[i].certificate_password = g_value_get_string(gda_data_model_get_value_at(data_model, 15, i, NULL));
    		else
    			account[i].certificate_password = NULL;

    		account[i].is_default = g_value_get_int(gda_data_model_get_value_at(data_model, 16, i, NULL));
    	}

    	struct MqttModel * model = malloc (sizeof (struct MqttModel));
    	model->account = account;
    	model->account_size = rows;
    	//g_object_unref (data_model);
    	return model;
    }
	//gda_data_model_dump (data_model, stdout);
}

static struct MqttModel * get_topics () {

	GdaDataModel *data_model;
	GdaSqlParser *parser;
	GdaStatement *stmt;
	GError *error = NULL;

	gchar sql [100] = {};
	strcat(sql, "SELECT topic_name, qos FROM topic WHERE topic_account=");
	char account_id_string [3];
	sprintf(account_id_string, "%d", get_default_account_id());
	strcat(sql, account_id_string);

	parser = g_object_get_data (G_OBJECT (cnc), "parser");
	stmt = gda_sql_parser_parse_string (parser, sql, NULL, NULL);
	data_model = gda_connection_statement_execute_select (cnc, stmt, NULL, &error);
	g_object_unref (stmt);
    if (!data_model)
    	g_error ("Could not get the contents of the 'topic' table: %s\n",
                         error && error->message ? error->message : "No detail");
    gint rows = gda_data_model_get_n_rows(data_model);
    //gda_data_model_dump (data_model, stdout);
    if(rows < 1) {
    	g_object_unref (data_model);
    	return NULL;
    } else {
    	struct CommonTopic * common_topic = malloc (sizeof (struct CommonTopic) * rows);
    	for(int i = 0; i < rows; i++) {
    		common_topic[i].topic_name = g_value_get_string(gda_data_model_get_value_at(data_model, 0, i, NULL));
    		common_topic[i].qos = g_value_get_int(gda_data_model_get_value_at(data_model, 1, i, NULL));
    	}

    	struct MqttModel * model = malloc (sizeof (struct MqttModel));
    	model->topic = common_topic;
    	model->topic_size = rows;
    	return model;
    }
}

static struct MqttModel * get_messages() {

	GdaDataModel *data_model;
	GdaSqlParser *parser;
	GdaStatement *stmt;
	GError *error = NULL;

	gchar sql [100] = {};
	strcat(sql, "SELECT topic_name, content, qos, is_incoming FROM message WHERE message_account=");
	char account_id_string [3];
	sprintf(account_id_string, "%d", get_default_account_id());
	strcat(sql, account_id_string);

	parser = g_object_get_data (G_OBJECT (cnc), "parser");
	stmt = gda_sql_parser_parse_string (parser, sql, NULL, NULL);
	data_model = gda_connection_statement_execute_select (cnc, stmt, NULL, &error);
	g_object_unref (stmt);
    if (!data_model)
    	g_error ("Could not get the contents of the 'message' table: %s\n",
                         error && error->message ? error->message : "No detail");
    gint rows = gda_data_model_get_n_rows(data_model);
    //gda_data_model_dump (data_model, stdout);
    if(rows < 1) {
    	g_object_unref (data_model);
    	return NULL;
    } else {
    	struct CommonPublish * common_publish = malloc (sizeof (struct CommonPublish) * rows);
    	for(int i = 0; i < rows; i++) {
    		common_publish[i].topic_name = g_value_get_string(gda_data_model_get_value_at(data_model, 0, i, NULL));
    		common_publish[i].content = g_value_get_string(gda_data_model_get_value_at(data_model, 1, i, NULL));
    		common_publish[i].qos = g_value_get_int(gda_data_model_get_value_at(data_model, 2, i, NULL));
    		common_publish[i].is_incoming = g_value_get_int(gda_data_model_get_value_at(data_model, 3, i, NULL));
    	}

    	struct MqttModel * model = malloc (sizeof (struct MqttModel));
    	model->common_publish = common_publish;
    	model->publish_size = rows;
    	return model;
    }
}

void save_changes (struct Account * account) {
	cnc = open_db_connection ();
	create_account_table_if_not_exist (cnc);
	insert_data(cnc, account);
}

void save_topic_to_db (const char * topic_name, int qos) {
	cnc = open_db_connection ();
	create_topic_table_if_not_exist (cnc);
	insert_topic_data(cnc, topic_name, qos);
}

void remove_topic_from_db(const char * topic_name) {
	//cnc = open_db_connection ();
	remove_topic(cnc, topic_name);
}

void save_message (const char * _content, const char * _topic_name, int _qos, int _retain, int _dup, int _is_incoming) {
	cnc = open_db_connection ();
	create_message_table_if_not_exist (cnc);
	insert_message_data(cnc, _content, _topic_name, _qos, _retain, _dup, _is_incoming);
}

void set_account_default(int id) {
	cnc = open_db_connection ();
	set_default_all ();
	set_default(id);

}

void remove_account_from_db(int id){
	cnc = open_db_connection ();
	remove_account(id);
}

struct MqttModel * get_topics_from_db() {

	return get_topics();

}

struct MqttModel * get_messages_from_db() {

	return get_messages();

}


struct MqttModel * get_accounts() {
	cnc = open_db_connection ();
	create_account_table_if_not_exist (cnc);
	create_topic_table_if_not_exist (cnc);
	create_message_table_if_not_exist (cnc);
	set_default_all ();
	return get_accounts_from_db(cnc);
}


