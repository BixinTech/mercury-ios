//
//  MCRMessage+Protocol.m
//  Mercury
//
//  Created by 姜腾 on 2019/10/26.
//

#import "MCRMessage+Protocol.h"
#import "MCRConstants.h"


@implementation MCRMessage (Protocol)
+ (instancetype)messageWithProtocol:(mercury::MessageProtocol)protocol {
    MCRMessage *message = [[MCRMessage alloc] init];
    message.command = (MCRCommand)protocol.GetCommand();
    std::vector<uint8_t> s = protocol.GetBodyData();
    message.body = [NSData dataWithBytes:s.data() length:s.size()];
    std::vector<mercury::ExtensionHeader_t> headers = protocol.GetExtensionHeaders();
    NSMutableArray<MCRHeader *> *headerList = [[NSMutableArray alloc] init];
    for (mercury::ExtensionHeader_t h : headers) {
        MCRHeader *mrch = [[MCRHeader alloc] init];
        mrch.type = (MCRHeaderType)h.headerType;
        mrch.data = [[NSData alloc] initWithBytes:h.headerData.data() length:h.headerData.size()];
        [headerList addObject:mrch];
    }
    message.headers = headerList;
    return message;
}

- (mercury::MessageProtocol)messageProtocol {
    mercury::MessageProtocol msg;
    mercury::Meta_t m;
    m.command = self.command;
//    m.flag = (uint8_t)mercury::MessageFlag::CUSTOM;
    msg.SetMeta(m);
    
    for (MCRHeader *h in self.headers) {
        const uint8_t* bytes = (const uint8_t*)h.data.bytes;
        msg.AddExtensionHeader(h.type, std::vector<uint8_t>(bytes, bytes + h.data.length));
    }
    
    if (self.body.length > 0) {
        const uint8_t* bytes = (const uint8_t*)self.body.bytes;
        msg.SetBodyData(std::vector<uint8_t>(bytes,bytes + self.body.length));
    }
    
    return msg;
}
@end
