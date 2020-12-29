/*
 * 通知实现
 */

#include "redis.h"

/* This file implements keyspace events notification via Pub/Sub ad
 * described at http://redis.io/topics/keyspace-events. */

/* Turn a string representing notification classes into an integer
 * representing notification classes flags xored.
 *
 * 对传入的字符串参数进行分析， 给出相应的 flags 值
 *
 * The function returns -1 if the input contains characters not mapping to
 * any class. 
 *
 * 如果传入的字符串中有不能识别的字符串，那么返回 -1 。
 */
int keyspaceEventsStringToFlags(char *classes) {
    char *p = classes;
    int c, flags = 0;

    while((c = *p++) != '\0') {
        switch(c) {
        case 'A': flags |= REDIS_NOTIFY_ALL; break;
        case 'g': flags |= REDIS_NOTIFY_GENERIC; break;
        case '$': flags |= REDIS_NOTIFY_STRING; break;
        case 'l': flags |= REDIS_NOTIFY_LIST; break;
        case 's': flags |= REDIS_NOTIFY_SET; break;
        case 'h': flags |= REDIS_NOTIFY_HASH; break;
        case 'z': flags |= REDIS_NOTIFY_ZSET; break;
        case 'x': flags |= REDIS_NOTIFY_EXPIRED; break;
        case 'e': flags |= REDIS_NOTIFY_EVICTED; break;
        case 'K': flags |= REDIS_NOTIFY_KEYSPACE; break;
        case 'E': flags |= REDIS_NOTIFY_KEYEVENT; break;
        // 不能识别
        default: return -1;
        }
    }

    return flags;
}

/* This function does exactly the revese of the function above: it gets
 * as input an integer with the xored flags and returns a string representing
 * the selected classes. The string returned is an sds string that needs to
 * be released with sdsfree(). */
/*
 * 根据 flags 值还原设置这个 flags 所需的字符串
 */
sds keyspaceEventsFlagsToString(int flags) {
    sds res;

    res = sdsempty();
    if ((flags & REDIS_NOTIFY_ALL) == REDIS_NOTIFY_ALL) {
        res = sdscatlen(res,"A",1);
    } else {
        if (flags & REDIS_NOTIFY_GENERIC) res = sdscatlen(res,"g",1);
        if (flags & REDIS_NOTIFY_STRING) res = sdscatlen(res,"$",1);
        if (flags & REDIS_NOTIFY_LIST) res = sdscatlen(res,"l",1);
        if (flags & REDIS_NOTIFY_SET) res = sdscatlen(res,"s",1);
        if (flags & REDIS_NOTIFY_HASH) res = sdscatlen(res,"h",1);
        if (flags & REDIS_NOTIFY_ZSET) res = sdscatlen(res,"z",1);
        if (flags & REDIS_NOTIFY_EXPIRED) res = sdscatlen(res,"x",1);
        if (flags & REDIS_NOTIFY_EVICTED) res = sdscatlen(res,"e",1);
    }
    if (flags & REDIS_NOTIFY_KEYSPACE) res = sdscatlen(res,"K",1);
    if (flags & REDIS_NOTIFY_KEYEVENT) res = sdscatlen(res,"E",1);

    return res;
}

/* The API provided to the rest of the Redis core is a simple function:
 * notifyKeyspaceEvent(char *event, robj *key, int dbid);
 * 'event' is a C string representing the event name.
 * event 参数是一个字符串表示的事件名
 * 'key' is a Redis object representing the key name.
 * key 参数是一个 Redis 对象表示的键名
 * 'dbid' is the database ID where the key lives.
 * dbid 参数为键所在的数据库
 */
void notifyKeyspaceEvent(int type, char *event, robj *key, int dbid) {
    sds chan;
    robj *chanobj, *eventobj;
    int len = -1;
    char buf[24];

    /* If notifications for this class of events are off, return ASAP. */
    // 如果服务器配置为不发送 type 类型的通知，那么直接返回
    if (!(server.notify_keyspace_events & type)) return;

    // 事件的名字
    eventobj = createStringObject(event,strlen(event));

    /* __keyspace@<db>__:<key> <event> notifications. */
    // 发送键空间通知
    if (server.notify_keyspace_events & REDIS_NOTIFY_KEYSPACE) {

        // 构建频道对象
        chan = sdsnewlen("__keyspace@",11);
        len = ll2string(buf,sizeof(buf),dbid);
        chan = sdscatlen(chan, buf, len);
        chan = sdscatlen(chan, "__:", 3);
        chan = sdscatsds(chan, key->ptr);

        chanobj = createObject(REDIS_STRING, chan);

        // 通过 publish 命令发送通知
        pubsubPublishMessage(chanobj, eventobj);

        // 释放频道对象
        decrRefCount(chanobj);
    }

    /* __keyevente@<db>__:<event> <key> notifications. */
    // 发送键事件通知
    if (server.notify_keyspace_events & REDIS_NOTIFY_KEYEVENT) {

        // 构建频道对象
        chan = sdsnewlen("__keyevent@",11);
        // 如果在前面发送键空间通知的时候计算了 len ，那么它就不会是 -1
        // 这可以避免计算两次 buf 的长度
        if (len == -1) len = ll2string(buf,sizeof(buf),dbid);
        chan = sdscatlen(chan, buf, len);
        chan = sdscatlen(chan, "__:", 3);
        chan = sdscatsds(chan, eventobj->ptr);

        chanobj = createObject(REDIS_STRING, chan);

        // 通过 publish 命令发送通知
        pubsubPublishMessage(chanobj, key);

        // 释放频道对象
        decrRefCount(chanobj);
    }

    // 释放事件对象
    decrRefCount(eventobj);
}
