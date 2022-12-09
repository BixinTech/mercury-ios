//
//  MCRMessage+MercuryMessage.m
//  Mercury
//
//  Created by 姜腾 on 2019/11/7.
//

#import "MCRMessage+MercuryMessage.h"
#import "MCRConstants.h"

@implementation MCRMessage (MercuryMessage)

+ (instancetype)messageWithCMessage:(mercury::TunnelMessage)msg {
    MCRMessage *message = [[MCRMessage alloc] init];
    message.command = (MCRCommand)msg.command;
    message.isTwoWay = msg.twoWay == 1;
    message.type = (NSInteger)msg.type;
    NSData *bodyData = [NSData dataWithBytes:msg.body.data() length:msg.body.size()];
    message.body = bodyData;
    
    NSMutableArray<MCRHeader *> *headerList = [[NSMutableArray alloc] init];
    for (mercury::TunnelMessage::TunnelMessageHeader h : msg.headerList) {
        MCRHeader *mrch = [[MCRHeader alloc] init];
        mrch.type = (MCRHeaderType)h.headerType;
        mrch.data = [[NSData alloc] initWithBytes:h.headerBody.data() length:h.headerBody.size()];
        [headerList addObject:mrch];
    }

    message.headers = headerList;
    return message;
}

- (mercury::TunnelMessage)cMessage {
    mercury::TunnelMessage msg;
    msg.command = (uint8_t)self.command;
    msg.twoWay = true;
    msg.type = MessageType::REQ;
    msg.compressType = mercury::CompressType::zip;
    
    if (self.body.length) {
        const uint8_t* bytes = (const uint8_t*)self.body.bytes;
        msg.body.assign(bytes, bytes + self.body.length);
    }
    
    for (MCRHeader *h in self.headers) {
        const uint8_t* bytes = (const uint8_t*)h.data.bytes;
        msg.AddHeader(h.type, std::vector<uint8_t>(bytes, bytes + h.data.length));
    }
    
    return msg;
}
@end
