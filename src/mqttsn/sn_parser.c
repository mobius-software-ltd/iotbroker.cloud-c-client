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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "packets/sn_message.h"
#include "packets/sn_advertise.h"
#include "packets/sn_search_gw.h"
#include "packets/sn_gw_info.h"
#include "packets/sn_connect.h"
#include "packets/sn_connack.h"
#include "packets/sn_will_topic_req.h"
#include "packets/sn_will_topic_resp.h"
#include "packets/sn_will_topic_upd.h"
#include "packets/sn_will_message_req.h"
#include "packets/sn_will_message_resp.h"
#include "packets/sn_will_message_upd.h"
#include "packets/sn_will_topic.h"
#include "packets/sn_will_message.h"
#include "packets/sn_register.h"
#include "packets/sn_reg_ack.h"
#include "packets/sn_publish.h"
#include "packets/sn_puback.h"
#include "packets/sn_pubrel.h"
#include "packets/sn_pubcomp.h"
#include "packets/sn_pubrec.h"
#include "packets/sn_subscribe.h"
#include "packets/sn_suback.h"
#include "packets/sn_unsubscribe.h"
#include "packets/sn_unsuback.h"
#include "packets/sn_pingreq.h"
#include "packets/sn_disconnect.h"
#include "packets/sn_encapsulated.h"
#include "packets/sn_topic.h"
#include "../helpers.h"

#define THREE_OCTET_LENGTH_SUFFIX 0x01
#define MQTT_SN_PROTOCOL_ID 1


enum Flag { DUPLICATE = 128, QOS_LEVEL_ONE = 96, QOS_2 = 64, QOS_1 = 32, RETAIN = 16, WILL = 8, CLEAN_SESSION = 4, RESERVED_TOPIC = 3, SHORT_TOPIC = 2, ID_TOPIC = 1 , NULLABLE = -1};

struct Flags {
	enum Flag dup;
	enum Flag qos_one;
	enum Flag qos1;
	enum Flag qos2;
	enum Flag retain;
	enum Flag will;
	enum Flag clean;
	enum Flag reserved;
	enum Flag short_topic;
	enum Flag id_topic;
};

struct ValidFlags {
	int dup;
	enum SnQoS qos;
	int retain;
	int will;
	int clean_session;
	enum SnTopicType topic_type;
};

static char * get_string(char * buf, int bytes_left) {
	char * string = malloc (sizeof (string)*(bytes_left+1));
	memcpy(string, buf, bytes_left);
	string[bytes_left] = '\0';
	return string;
}

int sn_get_length(struct SnMessage * sn_message) {
	int length = 0;

	switch (sn_message->message_type)
	{
		case SN_ADVERTISE: {
			length = 5;
	        break;
		}
	    case SN_SEARCHGW: {
	    	length = 3;
	        break;
	    }
	    case SN_GWINFO: {
	    	struct SnGwInfo gw_info = *(struct SnGwInfo*) sn_message->packet;
	    	length = 3;
	    	if (gw_info.gw_address != NULL)
	    	length += strlen(gw_info.gw_address);
	        break;
	    }
        case SN_CONNECT: {
        	struct SnConnect conn = *(struct SnConnect*) sn_message->packet;
        	if (conn.client_id == NULL || strlen(conn.client_id)==0)
        		printf("connect must contain a valid clientID");
        	length = 6 + strlen(conn.client_id);
            break;
        }
	    case SN_CONNACK: {
	    	length = 3;
            break;
	    }
	    case SN_WILL_TOPIC_REQ: {
	    	length = 2;
	    	break;
	    }
	    case SN_WILL_TOPIC: {
	    	struct SnWillTopic will_top = *(struct SnWillTopic*) sn_message->packet;
	    	length = 2;
			if (will_top.topic != NULL)
			{
				length += strlen(will_top.topic->value) + 1;
				if (strlen(will_top.topic->value) > 252)
					length += 2;
			}
	        break;
	    }
	    case SN_WILL_MSG_REQ: {
	    	length = 2;
	    	break;
	    }
	    case SN_WILL_MSG: {
	    	struct SnWillMessage will_msg = *(struct SnWillMessage*) sn_message->packet;
	    	length = 2;
			length += strlen(will_msg.message);
			if (strlen(will_msg.message) > 253)
				length += 2;
	    	break;
	    }
	    case SN_REGISTER: {
	    	struct SnRegister reg = *(struct SnRegister*) sn_message->packet;
	    	if (reg.topic_name == NULL) {
	    		printf("SN_REGISTER must contain a valid topic name\n");
	    		return -1;
	    	}
	    	length = 6;
	    	length += strlen(reg.topic_name);
	    	if (strlen(reg.topic_name) > 249)
	    		length += 2;
	    	break;
	    }
	    case SN_REGACK: {
	    	length = 7;
	    	break;
	    }
	    case SN_PUBLISH: {
	    	struct SnPublish pub = *(struct SnPublish*) sn_message->packet;
	    	length = 7;
			length += strlen(pub.data);
			if (strlen(pub.data) > 248)
				length += 2;
			break;
	    }
	    case SN_PUBACK: {
	    	length = 7;
	        break;
	    }
	    case SN_PUBREC: {
	    	length = 4;
	        break;
	    }
	    case SN_PUBREL: {
	    	length = 4;
	    	break;
	    }
	    case SN_PUBCOMP: {
	    	length = 4;
	    	break;
	    }
	    case SN_SUBSCRIBE: {
	    	struct SnSubscribe * sub = (struct SnSubscribe*) (sn_message->packet);
	    	length = 5;
	    	if(sub->topic->topic_type !=ID) {
	    		length += strlen(sub->topic->value);
	    		if (strlen(sub->topic->value) > 250)
	    			length += 2;
	    	} else {
	    		length += 2;
	    	}
	        break;
	    }
	    case SN_SUBACK: {
	    	length = 8;
	        break;
	    }
	    case SN_UNSUBSCRIBE: {
	    	struct SnUnsubscribe unsub = *(struct SnUnsubscribe*) sn_message->packet;
	    	length = 5;
	    	if(unsub.topic->topic_type !=ID) {
	    		length += strlen(unsub.topic->value);
	    		if (strlen(unsub.topic->value) > 250)
	    			length += 2;
	    	} else {
	    		length += 2;
	    	}
			break;
	    }
	    case SN_UNSUBACK: {
	    	length = 4;
	        break;
	    }
	    case SN_PINGREQ: {
	    	struct SnPingreq pingreq = *(struct SnPingreq*) sn_message->packet;
	    	length = 2;
	    	if (pingreq.client_id != NULL)
	    		length += strlen(pingreq.client_id);
	        break;
	    }
	    case SN_PINGRESP: {
	    	length = 2;
	        break;
	    }
	    case SN_DISCONNECT: {
	    	struct SnDisconnect dis = *(struct SnDisconnect*) sn_message->packet;
	    	length = 2;
	    	if (dis.duration > 0)
	    		length += 2;
	        break;
	    }
	    case SN_WILL_TOPIC_UPD: {
	    	struct SnWillTopicUpd top_upd = *(struct SnWillTopicUpd*) sn_message->packet;
	    	length = 2;
			if (top_upd.topic != NULL)
			{
				length += strlen(top_upd.topic->value) + 1;
				if (strlen(top_upd.topic->value) > 252)
					length += 2;
			}
			break;
	    }
	    case SN_WILL_MSG_UPD: {
	    	struct SnWillMsgUpd msg_upd = *(struct SnWillMsgUpd*) sn_message->packet;
	    	length = 2;
			length += strlen(msg_upd.message);
			if (strlen(msg_upd.message) > 253)
				length += 2;
	        break;
	    }
	    case SN_WILL_TOPIC_RESP: {
	    	length = 2;
	        break;
	    }
	    case SN_WILL_MSG_RESP: {
	    	length = 2;
	        break;
	    }
	    case SN_ENCAPSULATED: {
	    	struct SnEncapsulated en = *(struct SnEncapsulated*) sn_message->packet;
	    	length = 3;
			if (en.wireless_node_id != NULL)
				length += strlen(en.wireless_node_id);
			//FIXME
			if (en.message != NULL)
				length += strlen(en.message->packet);
			break;
	    }
		default: {
			printf("invalid packet type encoding: %i \n",sn_message->message_type);
			break;
		}
	}

	return length;
}

struct ValidFlags * validate_create_flags(struct Flags flags, enum SnMessageType type) {

	if (flags.reserved != -1)
		printf("Invalid topic type encoding. TopicType reserved bit must not be encoded\n");

	int dup = 0;
	if (flags.dup != -1)
		dup = 1;

	int retain = 0;
	if (flags.retain != -1)
		retain = 1;

	int will = 0;
	if (flags.will != -1)
		will = 1;

	int clean_session = 0;
	if (flags.clean != -1)
		clean_session = 1;

	enum SnQoS * qos =  malloc (sizeof (qos));
	if (flags.qos_one != -1) {
		*qos = SN_LEVEL_ONE;
	} else if(flags.qos2 != -1) {
		*qos = SN_EXACTLY_ONCE;
	} else if(flags.qos1 != -1) {
		*qos = SN_AT_LEAST_ONCE;
	} else {
		*qos = SN_AT_MOST_ONCE;
	}

	enum SnTopicType * topic_type = malloc (sizeof (topic_type));
	if (flags.short_topic != -1) {
		*topic_type = SHORT;
	} else if(flags.id_topic != -1) {
		*topic_type = ID;
	} else {
		*topic_type = NAMED;
	}

	switch (type) {
	case SN_CONNECT: {
		if (dup)
			printf("SN_CONNECT invalid encoding: dup flag\n");
		if (qos != SN_AT_MOST_ONCE)
			printf("SN_CONNECT  invalid encoding: qos flag - %i \n", *qos);
		if (retain)
			printf("SN_CONNECT  invalid encoding: retain flag\n");
		if (topic_type == NULL || *topic_type != NAMED)
			printf("SN_CONNECT  invalid encoding: topicIdType flag\n");
		break;
	}
	case SN_WILL_TOPIC: {
		if (dup)
			printf("SN_WILL_TOPIC invalid encoding: dup flag\n");
		if (qos == NULL)
			printf("SN_WILL_TOPIC invalid encoding: qos flag\n");
		if (will)
			printf("SN_WILL_TOPIC invalid encoding: will flag\n");
		if (clean_session)
			printf("SN_WILL_TOPIC invalid encoding: cleanSession flag\n");
		if (topic_type == NULL || *topic_type != NAMED)
			printf("SN_WILL_TOPIC invalid encoding: topicIdType flag\n");
		break;
	}
	case SN_PUBLISH: {
		if (qos == NULL)
			printf("SN_PUBLISH invalid encoding: qos flag\n");
		if (topic_type == NULL)
			printf("SN_PUBLISH invalid encoding: topicIdType flag\n");
		if (will)
			printf("SN_PUBLISH invalid encoding: will flag\n");
		if (clean_session)
			printf("SN_PUBLISH invalid encoding: cleanSession flag\n");
		if (dup && (*qos == SN_AT_MOST_ONCE || *qos == SN_LEVEL_ONE))
			printf(
					"SN_PUBLISH invalid encoding: dup flag with invalid qos: % i \n",
					*qos);
		break;
	}
	case SN_SUBSCRIBE: {
		if (qos == NULL)
			printf("SN_SUBSCRIBE invalid encoding: qos flag\n");
		if (*qos == SN_LEVEL_ONE)
			printf("SN_SUBSCRIBE invalid encoding: qos %i \n", *qos);
		if (retain)
			printf("SN_SUBSCRIBE invalid encoding: retain flag\n");
		if (will)
			printf("SN_SUBSCRIBE invalid encoding: will flag\n");
		if (clean_session)
			printf("SN_SUBSCRIBE invalid encoding: cleanSession flag\n");
		if (topic_type == NULL)
			printf("SN_SUBSCRIBE invalid encoding: retain flag\n");
		break;
	}
	case SN_SUBACK: {
		if (dup)
			printf("SN_SUBACK invalid encoding: dup flag\n");
		if (qos == NULL)
			printf("SN_SUBACK invalid encoding: qos flag\n");
		if (retain)
			printf("SN_SUBACK invalid encoding: retain flag\n");
		if (will)
			printf("SN_SUBACK invalid encoding: will flag\n");
		if (clean_session)
			printf("SN_SUBACK invalid encoding: cleanSession flag\n");
		if (topic_type == NULL || *topic_type != NAMED)
			printf("SN_SUBACK invalid encoding: topicIdType flag\n");
		break;
	}
	case SN_UNSUBSCRIBE: {
		if (dup)
			printf("SN_UNSUBSCRIBE invalid encoding: dup flag\n");
		if (qos != SN_AT_MOST_ONCE)
			printf("SN_UNSUBSCRIBE invalid encoding: qos flag\n");
		if (retain)
			printf("SN_UNSUBSCRIBE invalid encoding: retain flag\n");
		if (will)
			printf("SN_UNSUBSCRIBE invalid encoding: will flag\n");
		if (clean_session)
			printf("SN_UNSUBSCRIBE invalid encoding: cleanSession flag\n");
		if (topic_type == NULL)
			printf("SN_UNSUBSCRIBE invalid encoding: topicIdType flag\n");
		break;
	}
	case SN_WILL_TOPIC_UPD: {
		if (dup)
			printf("SN_WILL_TOPIC_UPD invalid encoding: dup flag\n");
		if (qos == NULL)
			printf("SN_WILL_TOPIC_UPD invalid encoding: qos flag\n");
		if (will)
			printf("SN_WILL_TOPIC_UPD invalid encoding: will flag\n");
		if (clean_session)
			printf("SN_WILL_TOPIC_UPD invalid encoding: cleanSession flag\n");
		if (topic_type == NULL || *topic_type != NAMED)
			printf("SN_WILL_TOPIC_UPD invalid encoding: topicIdType flag\n");
		break;
	}
	default:
		break;
	}
	struct ValidFlags * valid_flags = malloc (sizeof (struct ValidFlags));
	valid_flags->dup = dup;
	valid_flags->qos = *qos;
	valid_flags->retain = retain;
	valid_flags->will = will;
	valid_flags->clean_session = clean_session;
	valid_flags->topic_type = *topic_type;
	return valid_flags;
}

struct ValidFlags * decode_flag(char flagsByte, enum SnMessageType type) {

	struct Flags flags = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
	for(int i = ID_TOPIC; i <= DUPLICATE; i++ )
	{
		switch (i) {
			case ID_TOPIC : {
				if((flagsByte & i) == i)
					flags.id_topic = i;
				break;
			}
			case SHORT_TOPIC : {
				if((flagsByte & i) == i)
					flags.short_topic = i;
				break;
			}
			case RESERVED_TOPIC : {
				if((flagsByte & i) == i)
					flags.reserved = i;
				break;
			}
			case CLEAN_SESSION : {
				if((flagsByte & i) == i)
					flags.clean = i;
				break;
			}
			case WILL : {
				if((flagsByte & i) == i)
					flags.will = i;
				break;
			}
			case RETAIN : {
				if((flagsByte & i) == i)
					flags.retain = i;
				break;
			}
			case QOS_1 : {
				if((flagsByte & i) == i)
					flags.qos1 = i;
				break;
			}
			case QOS_2 : {
				if((flagsByte & i) == i)
					flags.qos2 = i;
				break;
			}
			case QOS_LEVEL_ONE : {
				if((flagsByte & i) == i)
					flags.qos_one = i;
				break;
			}
			case DUPLICATE : {
				if((flagsByte & i) == i)
					flags.dup = i;
				break;
			}
		}
	}
	return validate_create_flags(flags, type);
}



char encode_flags(int dup, enum SnQoS * qos, int retain, int will, int cleanSession, enum SnTopicType * topicType) {

	char flagsByte = 0;
	if(dup)
		flagsByte += DUPLICATE;

	if(qos != NULL)
		flagsByte += *(qos) << 5;

	if(retain)
		flagsByte += RETAIN;

	if(will)
		flagsByte += WILL;

	if(cleanSession)
		flagsByte += CLEAN_SESSION;

	if(topicType != NULL)
		flagsByte += *topicType;

	return flagsByte;
}


char * sn_encode(struct SnMessage * message, int length) {

	 char * buf = NULL;
	 buf = malloc(sizeof(char) * length);
	 unsigned short string_length = 0;
	 int i = 0;
	 if (length <= 255)
		 buf[i++] = (char)length;
	 else
	 {
		 buf[i++] = THREE_OCTET_LENGTH_SUFFIX;
		 i += add_short(&buf[i], length);
	 }

	 buf[i++] = message->message_type;

	 switch (message->message_type)
	 {
	 	 case SN_ADVERTISE: {
	 		struct SnAdvertise adv = *(struct SnAdvertise*) message->packet;
	 		buf[i++] = adv.gw_id;
	 		add_short(&buf[i], adv.duration);
	 		break;
	 	 }
	     case SN_SEARCHGW: {
	    	struct SnSearchGw ssg = *(struct SnSearchGw*) message->packet;
	        buf[i] = ssg.radius;
	        break;
	     }
	     case SN_GWINFO: {
	    	struct SnGwInfo sgi = *(struct SnGwInfo*) message->packet;
	    	buf[i++] = sgi.gw_id;
	    	if(sgi.gw_address != NULL) {
	    		string_length = strlen(sgi.gw_address);
	    		if (string_length != 0) {
	    		 	memcpy(&buf[i], sgi.gw_address, string_length);
   		 		}
	    	 }
             break;
	     }
         case SN_CONNECT: {
        	 struct SnConnect sc = *(struct SnConnect*) message->packet;
        	 buf[i++] = encode_flags(0, NULL, 0, sc.will_present, sc.clean_session, NULL);
        	 buf[i++] = sc.protocol_id;
        	 i += add_short(&buf[i], sc.duration);
        	 string_length = strlen(sc.client_id);
        	 if (string_length != 0) {
        		 memcpy(&buf[i], sc.client_id, string_length);
        		 i += string_length;
        	 }
        	 break;
         }
	     case SN_CONNACK: {
	    	 struct SnConnack sca = *(struct SnConnack*) message->packet;
	    	 buf[i] = sca.return_code;
	    	 break;
	     }
	     case SN_WILL_TOPIC_RESP: {
	    	 struct SnWillTopicResp swtrs = *(struct SnWillTopicResp*) message->packet;
	    	 buf[i] = swtrs.return_code;
	    	 break;
	     }
	     case SN_WILL_MSG_RESP: {
	    	 struct SnWillMsgResp swmrs = *(struct SnWillMsgResp*) message->packet;
	    	 buf[i] = swmrs.return_code;
	    	 break;
	     }
	     case SN_WILL_TOPIC: {
	    	 struct SnWillTopic swt = *(struct SnWillTopic*) message->packet;
	    	 if(swt.topic != NULL) {
	    		 buf[i++] = encode_flags(0, &swt.topic->qos, swt.retain, 0, 0, NULL);
	    		 string_length = strlen(swt.topic->value);
	    		 if (string_length != 0) {
	    			 memcpy(&buf[i], swt.topic->value, string_length);
	    			 i += string_length;
	    		 }
	    	 }

	    	 break;
	     }
	     case SN_WILL_MSG: {
	    	 struct SnWillMessage swm = *(struct SnWillMessage*) message->packet;
    		 string_length = strlen(swm.message);
    		 if (string_length != 0) {
    			 memcpy(&buf[i], swm.message, string_length);
    			 i += string_length;
    		 }
    		 break;
	     }
         case SN_REGISTER: {
        	 struct SnRegister sr = *(struct SnRegister*) message->packet;
        	 i += add_short(&buf[i], sr.topic_id);
        	 i += add_short(&buf[i], sr.msg_id);
        	 string_length = strlen(sr.topic_name);
        	 if (string_length != 0) {
        		 memcpy(&buf[i], sr.topic_name, string_length);
        		 i += string_length;
        	 }
        	 break;
         }
	     case SN_REGACK: {
	    	 struct SnRegAck sra = *(struct SnRegAck*) message->packet;
        	 i += add_short(&buf[i], sra.topic_id);
        	 i += add_short(&buf[i], sra.message_id);
        	 buf[i] = sra.code;
        	 break;
	     }
	     case SN_PUBLISH: {
	    	 struct SnPublish sp = *(struct SnPublish*) message->packet;
	    	 buf[i++] = encode_flags(sp.dup, &(sp.topic.qos), sp.retain, 0, 0, &(sp.topic.topic_type));
	    	 if(sp.topic.topic_type == ID) {
	    		 i += add_short(&buf[i], sp.topic.id);
	    	 } else {
	    		 string_length = strlen(sp.topic.value);
	    		 if (string_length != 0) {
	    			 memcpy(&buf[i], sp.topic.value, string_length);
	    			 i += string_length;
	    		 }
	    	 }
	    	 i += add_short(&buf[i], sp.message_id);

	    	 string_length = strlen(sp.data);
	    	 if (string_length != 0) {
	    		 memcpy(&buf[i], sp.data, string_length);
	    		 i+=string_length;
	    	 }

	    	 break;
	     }
	     case SN_PUBACK: {
	    	 struct SnPuback spa = *(struct SnPuback*) message->packet;
	    	 i += add_short(&buf[i], spa.topic_id);
	    	 i += add_short(&buf[i], spa.message_id);
	    	 buf[i] = spa.code;
             break;
	     }
	     case SN_PUBREC: {
	    	 struct SnPubrec spc = *(struct SnPubrec*) message->packet;
	    	 add_short(&buf[i], spc.message_id);
	    	 break;
	     }
	     case SN_PUBREL: {
	    	 struct SnPubrel spr = *(struct SnPubrel*) message->packet;
	    	 add_short(&buf[i], spr.message_id);
	    	 break;
	     }
	     case SN_PUBCOMP: {
	    	 struct SnPubcomp spcp = *(struct SnPubcomp*) message->packet;
	    	 add_short(&buf[i], spcp.message_id);
	    	 break;
	     }
	     case SN_UNSUBACK: {
	    	 struct SnUnsuback susa = *(struct SnUnsuback*) message->packet;
	    	 add_short(&buf[i], susa.message_id);
	    	 break;
	     }
         case SN_SUBSCRIBE: {
        	 struct SnSubscribe ss = *(struct SnSubscribe*) message->packet;
        	 buf[i++] = encode_flags(ss.dup, &(ss.topic->qos), 0, 0, 0, &(ss.topic->topic_type));
        	 i += add_short(&buf[i], ss.message_id);
        	 if(ss.topic->topic_type == ID) {
        		 i += add_short(&buf[i], ss.topic->id);
        	 } else {
        		 string_length = strlen(ss.topic->value);
        		 if (string_length != 0) {
        			 memcpy(&buf[i], ss.topic->value, string_length);
        		 }
        	 }
        	 i += string_length;
        	 break;
         }
	     case SN_SUBACK: {
	    	 struct SnSuback ssa = *(struct SnSuback*) message->packet;
	    	 buf[i++] = encode_flags(0, &(ssa.allowedQos), 0, 0, 0, NULL);
	    	 i += add_short(&buf[i], ssa.topic_id);
	    	 i += add_short(&buf[i], ssa.message_id);
	    	 buf[i] = ssa.code;
	    	 break;
	     }
	     case SN_UNSUBSCRIBE: {
	    	 struct SnUnsubscribe sus = *(struct SnUnsubscribe*) message->packet;
	    	 buf[i++] = encode_flags(0, NULL, 0, 0, 0, &(sus.topic->topic_type));
	    	 i += add_short(&buf[i], sus.message_id);
	    	 if(sus.topic->topic_type == ID) {
	    		 i += add_short(&buf[i], sus.topic->id);
	    	 } else {
	    		 string_length = strlen(sus.topic->value);
	    		 if (string_length != 0) {
	    			 memcpy(&buf[i], sus.topic->value, string_length);
	    			 i += string_length;
	    		 }
	    	 }
	    	 break;
	     }
	     case SN_PINGREQ: {
	    	 if (length > 2)
	    	 {
	    		 struct SnPingreq spr = *(struct SnPingreq*) message->packet;
	    		 string_length = strlen(spr.client_id);
	    		 if (string_length != 0) {
	    			 memcpy(&buf[i], spr.client_id, string_length);
	    		 }
	    		 i += string_length;
	    	 }
	    	 break;
	     }
	     case SN_DISCONNECT: {
	    	 if (length > 2)
	    	 {
	    		 struct SnDisconnect sd = *(struct SnDisconnect*) message->packet;
	    		 i += add_short(&buf[i], sd.duration);
	    	 }
	    	 break;
	     }
	     case SN_WILL_TOPIC_UPD: {
	    	 struct SnWillTopicUpd swtu = *(struct SnWillTopicUpd*) message->packet;
	    	 if (swtu.topic != NULL)
	    	 {
	    		 buf[i++] = encode_flags(0, &(swtu.topic->qos), swtu.retain, 0, 0, NULL);
	    		 string_length = strlen(swtu.topic->value);
	    		 if (string_length != 0) {
	    			 memcpy(&buf[i], swtu.topic->value, string_length);
	    		 }
	    	 }
	    	 break;
	     }
	     case SN_WILL_MSG_UPD: {
	    	 struct SnWillMsgUpd swmu = *(struct SnWillMsgUpd*) message->packet;
	    	 string_length = strlen(swmu.message);
	    	 if (string_length != 0) {
	    		 memcpy(&buf[i], swmu.message, string_length);
	    	 }
	    	 break;
	     }
	     case SN_WILL_TOPIC_REQ:
	     case SN_WILL_MSG_REQ:
	     case SN_PINGRESP:
	     break;

	     case SN_ENCAPSULATED: {
	    	 struct SnEncapsulated se = *(struct SnEncapsulated*) message->packet;
	    	 buf[i++] = se.radius;
	    	 string_length = strlen(se.wireless_node_id);
	    	 if (string_length != 0) {
	    		 memcpy(&buf[i], se.wireless_node_id, string_length);
	    		 i += string_length;
	    	 }
	    	 //FIXME add encapsulated message
	    	 //sn_encode(se.message, );
	         break;
	     }
	     default:
	    	 break;
	 }

	 if (message->message_type != SN_ENCAPSULATED && length != i) {
		 printf("invalid message encoding: expected length-%i, actual-%i \n",length, i);
	 }
	 return buf;
}

struct SnMessage * sn_decode(char * buf) {
	struct SnMessage * m = malloc (sizeof (struct SnMessage));

	int i = 0;
	int length = 0;
	short first_length_byte = (short)(0x00FF & (short)buf[i++]);
	if (first_length_byte == THREE_OCTET_LENGTH_SUFFIX) {
		add_short(&buf[i], length);
		i+=2;
	}
	else {
		length = first_length_byte;
	}

	int bytes_left = length - i;

	enum SnMessageType type = buf[i++];
	m->message_type = type;

	bytes_left--;
	switch (type)
	{
		case SN_ADVERTISE: {
			struct SnAdvertise * adv = malloc (sizeof (struct SnAdvertise));
			adv->gw_id = buf[i++];
			adv->duration = get_short(buf, i);
			//i += 2;
			m->packet = adv;
	        break;
		}
	    case SN_SEARCHGW: {
	    	struct SnSearchGw * search = malloc (sizeof (struct SnSearchGw));
	    	search->radius = buf[i++];
	    	m->packet = search;
	        break;
	    }
	    case SN_GWINFO: {
	    	struct SnGwInfo * gw_info = malloc (sizeof (struct SnGwInfo));
	    	gw_info->gw_id = buf[i++];
	    	bytes_left--;
	    	gw_info->gw_address = get_string(&buf[i], bytes_left);
	        m->packet = gw_info;
	        break;
	    }
        case SN_CONNECT: {
        	struct SnConnect * conn = malloc (sizeof (struct SnConnect));
        	struct ValidFlags * connect_flags = decode_flag(buf[i++], type);
        	conn->will_present = connect_flags->will;
        	conn->clean_session = connect_flags->clean_session;
        	bytes_left--;
        	conn->protocol_id = buf[i++];
	        bytes_left--;
	        if (conn->protocol_id != MQTT_SN_PROTOCOL_ID)
	        	printf("Invalid protocolID: %i \n", conn->protocol_id);
	        conn->duration = get_short(buf, i);
	        i += 2;
            bytes_left -= 2;
            conn->client_id = get_string(&buf[i], bytes_left);
            m->packet = conn;
            break;
        }
	    case SN_CONNACK: {
	    	struct SnConnack * connack = malloc (sizeof (struct SnConnack));
	    	connack->return_code = buf[i];
            m->packet = connack;
            break;
	    }
	    case SN_WILL_TOPIC_REQ: {
	    	m->packet = NULL;
	    	break;
	    }
	    case SN_WILL_TOPIC: {
	    	struct SnWillTopic * will_topic = malloc (sizeof (struct SnWillTopic));
	    	struct ValidFlags * will_topic_flags = decode_flag(buf[i++], type);
	    	will_topic->retain = will_topic_flags->retain;
	    	will_topic->topic = NULL;
	        if (bytes_left > 0)
	        {
	        	will_topic->topic = malloc (sizeof (struct SnTopic));
	        	struct ValidFlags * will_topic_flags = decode_flag(buf[i++], type);
	        	bytes_left--;
	        	will_topic->retain = will_topic_flags->retain;
	        	will_topic->topic->value = get_string(&buf[i], bytes_left);
	        	will_topic->topic->qos = will_topic_flags->qos;
	        }
	        m->packet = will_topic;
	        break;
	    }
	    case SN_WILL_MSG_REQ: {
	    	m->packet = NULL;
	    	break;
	    }
	    case SN_WILL_MSG: {
	    	struct SnWillMessage * will_message = malloc (sizeof (struct SnWillMessage));
	    	will_message->message = get_string(&buf[i], bytes_left);
	    	m->packet = will_message;
	    	break;
	    }
	    case SN_REGISTER: {
	    	struct SnRegister * reg = malloc (sizeof (struct SnRegister));
	    	reg->topic_id = get_short(buf, i);
	    	i += 2;
	    	bytes_left -= 2;
	    	reg->msg_id = get_short(buf, i);
	    	i += 2;
	    	bytes_left -= 2;
	    	reg->topic_name = get_string(&buf[i], bytes_left);
	    	m->packet = reg;
	    	break;
	    }
	    case SN_REGACK: {
	    	struct SnRegAck * regack = malloc (sizeof (struct SnRegAck));
	    	regack->topic_id = get_short(buf, i);
	    	i += 2;
	    	regack->message_id = get_short(buf, i);
	    	i += 2;
	    	regack->code = buf[i];
	    	m->packet = regack;
	    	break;
	    }
	    case SN_PUBLISH: {
	    	struct SnPublish * pub = malloc (sizeof (struct SnPublish));
	    	struct ValidFlags * pub_flags = decode_flag(buf[i++], type);
	    	pub->dup = pub_flags->dup;
	    	pub->retain = pub_flags->retain;
	    	bytes_left--;
	    	unsigned short topic_id = get_short(buf, i);
	    	i += 2;
	    	bytes_left -= 2;
	    	pub->message_id = get_short(buf, i);
	    	i += 2;
	    	bytes_left -= 2;

	    	if (pub_flags->qos != SN_AT_MOST_ONCE && pub->message_id == 0)
	    		printf("invalid PUBLISH QoS-0 messageID: %i \n", pub->message_id);

            if (pub_flags->topic_type == SHORT)
            {
            	char str[6];
            	sprintf(str, "%d", topic_id);
            	pub->topic.value = str;
            }
	        else
	        {
	        	pub->topic.id = topic_id;
	        }
            pub->topic.qos = pub_flags->qos;
            if (bytes_left > 0) {
            	pub->data = get_string(&buf[i], bytes_left);
            }
            m->packet = pub;
            break;
	    }
	    case SN_PUBACK: {
	    	struct SnPuback * puback = malloc (sizeof (struct SnPuback));
	    	puback->topic_id = get_short(buf, i);
	    	i += 2;
	    	puback->message_id = get_short(buf, i);
	    	i += 2;
	    	puback->code = buf[i];
	    	m->packet = puback;
	        break;
	    }
	    case SN_PUBREC: {
	    	struct SnPubrec * pubrec = malloc (sizeof (struct SnPubrec));
	    	pubrec->message_id = get_short(buf, i);
	    	m->packet = pubrec;
	        break;
	    }
	    case SN_PUBREL: {
	    	struct SnPubrel * pubrel = malloc (sizeof (struct SnPubrel));
	    	pubrel->message_id = get_short(buf, i);
	    	m->packet = pubrel;
	    	break;
	    }
	    case SN_PUBCOMP: {
	    	struct SnPubcomp * pubcomp = malloc (sizeof (struct SnPubcomp));
	    	pubcomp->message_id = get_short(buf, i);
	    	m->packet = pubcomp;
	    	break;
	    }
	    case SN_SUBSCRIBE: {
	    	struct SnSubscribe * sub = malloc (sizeof (struct SnSubscribe));
	    	struct ValidFlags * sub_flags = decode_flag(buf[i++], type);
	    	sub->dup = sub_flags->dup;
	    	sub->topic->topic_type = sub_flags->topic_type;
	        bytes_left--;
	        sub->message_id = get_short(buf, i);
	        i += 2;
	        bytes_left -= 2;
	        if (bytes_left < 2)
	            printf("SN_SUBSCRIBE invalid topic encoding \n");

	        switch (sub_flags->topic_type)
			{
				case NAMED:
					sub->topic->value = get_string(&buf[i], bytes_left);
					break;
				case ID:
					sub->topic->id = get_short(buf, i);
					break;
				case SHORT:
					sub->topic->value = get_string(&buf[i], bytes_left);
					break;
			}
	        sub->topic->qos = sub_flags->qos;
	        m->packet = sub;
	        break;
	    }
	    case SN_SUBACK: {
	    	struct SnSuback * suback = malloc (sizeof (struct SnSuback));
	    	struct ValidFlags * sub_flags = malloc (sizeof (struct ValidFlags));
	    	sub_flags =	decode_flag(buf[i++], type);
	    	//i++;
	    	suback->allowedQos = sub_flags->qos;
	    	suback->topic_id = get_short(buf, i);
	    	i += 2;
	    	suback->message_id = get_short(buf, i);
	    	i += 2;
	    	suback->code = buf[i];
	    	m->packet = suback;
	        break;
	    }
	    case SN_UNSUBSCRIBE: {
	    	struct SnUnsubscribe * unsub = malloc (sizeof (struct SnUnsubscribe));
	    	struct ValidFlags * unsub_flags = decode_flag(buf[i++], type);
	    	unsub->topic->qos = unsub_flags->qos;
	    	unsub->topic->topic_type = unsub_flags->topic_type;
	    	bytes_left--;
	    	unsub->message_id = get_short(buf, i);
	    	i += 2;
	    	bytes_left -= 2;

			switch (unsub_flags->topic_type)
			{
				case NAMED:
					unsub->topic->value = get_string(&buf[i], bytes_left);
					break;
				case ID:
					unsub->topic->id = get_short(buf, i);
					break;
				case SHORT:
					unsub->topic->value = get_string(&buf[i], bytes_left);
					break;
			}
			m->packet = unsub;
			break;
	    }
	    case SN_UNSUBACK: {
	    	struct SnUnsuback * unsuback = malloc (sizeof (struct SnUnsuback));
	    	unsuback->message_id = get_short(buf, i);
	    	m->packet = unsuback;
	        break;
	    }
	    case SN_PINGREQ: {
	    	struct SnPingreq * pingreq = malloc (sizeof (struct SnPingreq));
	    	pingreq->client_id = NULL;
	    	if (bytes_left > 0)
	    		pingreq->client_id = get_string(&buf[i], bytes_left);
	    	m->packet = pingreq;
	        break;
	    }
	    case SN_PINGRESP: {
	    	m->packet = NULL;
	        break;
	    }
	    case SN_DISCONNECT: {
	    	struct SnDisconnect * disc = malloc (sizeof (struct SnDisconnect));
	        disc->duration = 0;
	        if (bytes_left > 0)
	        	disc->duration = get_short(buf, i);
	        m->packet = disc;
	        break;
	    }
	    case SN_WILL_TOPIC_UPD: {
	    	struct SnWillTopicUpd * will_topic_upd = malloc (sizeof (struct SnWillTopicUpd));
	    	will_topic_upd->retain = 0;
	    	will_topic_upd->topic = NULL;

	    	if (bytes_left > 0)
			{
	    		struct ValidFlags * will_topic_upd_flags = decode_flag(buf[i++], type);
	    		will_topic_upd->retain = will_topic_upd_flags->retain;
	    		will_topic_upd->topic->qos = will_topic_upd_flags->qos;
				bytes_left--;
				will_topic_upd->topic->value = get_string(&buf[i], bytes_left);
			}
	    	m->packet = will_topic_upd;
			break;
	    }
	    case SN_WILL_MSG_UPD: {
	    	struct SnWillMsgUpd * will_msg_upd = malloc (sizeof (struct SnWillMsgUpd));
	    	will_msg_upd->message = get_string(&buf[i], bytes_left);
	    	m->packet = will_msg_upd;
	        break;
	    }
	    case SN_WILL_TOPIC_RESP: {
	    	struct SnWillTopicResp * will_topic_resp = malloc (sizeof (struct SnWillTopicResp));
	    	will_topic_resp->return_code = buf[i];
	    	m->packet = will_topic_resp;
	        break;
	    }
	    case SN_WILL_MSG_RESP: {
	    	struct SnWillMsgResp * will_msg_resp = malloc (sizeof (struct SnWillMsgResp));
	    	will_msg_resp->return_code = buf[i];
	    	m->packet = will_msg_resp;
	        break;
	    }
	    case SN_ENCAPSULATED: {
	    	//TODO
			break;
	    }
		default: {
			printf("invalid packet type encoding: %i \n",type);
			break;
		}
	}
	return m;
}
