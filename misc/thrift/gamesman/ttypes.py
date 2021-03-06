#
# Autogenerated by Thrift
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#

from thrift.Thrift import *

from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol
try:
  from thrift.protocol import fastbinary
except:
  fastbinary = None


class GamestateResponse(object):
  """
  A structured representation of the server response, which directly maps
  to the JSON string returned by the GamesmanWeb server.
  
  Attributes:
   - board: The following fields apply to all move values, including those returned
  by getMoveValue().
   - remoteness
   - value
   - move: getNextMoveValues() also includes the moves that specify how to get
  from a parent board to a child.
   - score: For certain games, we may also need to set the following fields
  """

  thrift_spec = (
    None, # 0
    (1, TType.STRING, 'board', None, None, ), # 1
    (2, TType.I32, 'remoteness', None, None, ), # 2
    (3, TType.STRING, 'value', None, None, ), # 3
    (4, TType.STRING, 'move', None, None, ), # 4
    (5, TType.I32, 'score', None, None, ), # 5
  )

  def __init__(self, board=None, remoteness=None, value=None, move=None, score=None,):
    self.board = board
    self.remoteness = remoteness
    self.value = value
    self.move = move
    self.score = score

  def read(self, iprot):
    if iprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and isinstance(iprot.trans, TTransport.CReadableTransport) and self.thrift_spec is not None and fastbinary is not None:
      fastbinary.decode_binary(self, iprot.trans, (self.__class__, self.thrift_spec))
      return
    iprot.readStructBegin()
    while True:
      (fname, ftype, fid) = iprot.readFieldBegin()
      if ftype == TType.STOP:
        break
      if fid == 1:
        if ftype == TType.STRING:
          self.board = iprot.readString();
        else:
          iprot.skip(ftype)
      elif fid == 2:
        if ftype == TType.I32:
          self.remoteness = iprot.readI32();
        else:
          iprot.skip(ftype)
      elif fid == 3:
        if ftype == TType.STRING:
          self.value = iprot.readString();
        else:
          iprot.skip(ftype)
      elif fid == 4:
        if ftype == TType.STRING:
          self.move = iprot.readString();
        else:
          iprot.skip(ftype)
      elif fid == 5:
        if ftype == TType.I32:
          self.score = iprot.readI32();
        else:
          iprot.skip(ftype)
      else:
        iprot.skip(ftype)
      iprot.readFieldEnd()
    iprot.readStructEnd()

  def write(self, oprot):
    if oprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and self.thrift_spec is not None and fastbinary is not None:
      oprot.trans.write(fastbinary.encode_binary(self, (self.__class__, self.thrift_spec)))
      return
    oprot.writeStructBegin('GamestateResponse')
    if self.board != None:
      oprot.writeFieldBegin('board', TType.STRING, 1)
      oprot.writeString(self.board)
      oprot.writeFieldEnd()
    if self.remoteness != None:
      oprot.writeFieldBegin('remoteness', TType.I32, 2)
      oprot.writeI32(self.remoteness)
      oprot.writeFieldEnd()
    if self.value != None:
      oprot.writeFieldBegin('value', TType.STRING, 3)
      oprot.writeString(self.value)
      oprot.writeFieldEnd()
    if self.move != None:
      oprot.writeFieldBegin('move', TType.STRING, 4)
      oprot.writeString(self.move)
      oprot.writeFieldEnd()
    if self.score != None:
      oprot.writeFieldBegin('score', TType.I32, 5)
      oprot.writeI32(self.score)
      oprot.writeFieldEnd()
    oprot.writeFieldStop()
    oprot.writeStructEnd()

  def __repr__(self):
    L = ['%s=%r' % (key, value)
      for key, value in self.__dict__.iteritems()]
    return '%s(%s)' % (self.__class__.__name__, ', '.join(L))

  def __eq__(self, other):
    return isinstance(other, self.__class__) and self.__dict__ == other.__dict__

  def __ne__(self, other):
    return not (self == other)

class GetNextMoveResponse(object):
  """
  Attributes:
   - status
   - response
   - message
  """

  thrift_spec = (
    None, # 0
    (1, TType.STRING, 'status', None, None, ), # 1
    (2, TType.LIST, 'response', (TType.STRUCT,(GamestateResponse, GamestateResponse.thrift_spec)), None, ), # 2
    (3, TType.STRING, 'message', None, None, ), # 3
  )

  def __init__(self, status=None, response=None, message=None,):
    self.status = status
    self.response = response
    self.message = message

  def read(self, iprot):
    if iprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and isinstance(iprot.trans, TTransport.CReadableTransport) and self.thrift_spec is not None and fastbinary is not None:
      fastbinary.decode_binary(self, iprot.trans, (self.__class__, self.thrift_spec))
      return
    iprot.readStructBegin()
    while True:
      (fname, ftype, fid) = iprot.readFieldBegin()
      if ftype == TType.STOP:
        break
      if fid == 1:
        if ftype == TType.STRING:
          self.status = iprot.readString();
        else:
          iprot.skip(ftype)
      elif fid == 2:
        if ftype == TType.LIST:
          self.response = []
          (_etype3, _size0) = iprot.readListBegin()
          for _i4 in xrange(_size0):
            _elem5 = GamestateResponse()
            _elem5.read(iprot)
            self.response.append(_elem5)
          iprot.readListEnd()
        else:
          iprot.skip(ftype)
      elif fid == 3:
        if ftype == TType.STRING:
          self.message = iprot.readString();
        else:
          iprot.skip(ftype)
      else:
        iprot.skip(ftype)
      iprot.readFieldEnd()
    iprot.readStructEnd()

  def write(self, oprot):
    if oprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and self.thrift_spec is not None and fastbinary is not None:
      oprot.trans.write(fastbinary.encode_binary(self, (self.__class__, self.thrift_spec)))
      return
    oprot.writeStructBegin('GetNextMoveResponse')
    if self.status != None:
      oprot.writeFieldBegin('status', TType.STRING, 1)
      oprot.writeString(self.status)
      oprot.writeFieldEnd()
    if self.response != None:
      oprot.writeFieldBegin('response', TType.LIST, 2)
      oprot.writeListBegin(TType.STRUCT, len(self.response))
      for iter6 in self.response:
        iter6.write(oprot)
      oprot.writeListEnd()
      oprot.writeFieldEnd()
    if self.message != None:
      oprot.writeFieldBegin('message', TType.STRING, 3)
      oprot.writeString(self.message)
      oprot.writeFieldEnd()
    oprot.writeFieldStop()
    oprot.writeStructEnd()

  def __repr__(self):
    L = ['%s=%r' % (key, value)
      for key, value in self.__dict__.iteritems()]
    return '%s(%s)' % (self.__class__.__name__, ', '.join(L))

  def __eq__(self, other):
    return isinstance(other, self.__class__) and self.__dict__ == other.__dict__

  def __ne__(self, other):
    return not (self == other)

class GetMoveResponse(object):
  """
  Attributes:
   - status
   - response
   - message
  """

  thrift_spec = (
    None, # 0
    (1, TType.STRING, 'status', None, None, ), # 1
    (2, TType.STRUCT, 'response', (GamestateResponse, GamestateResponse.thrift_spec), None, ), # 2
    (3, TType.STRING, 'message', None, None, ), # 3
  )

  def __init__(self, status=None, response=None, message=None,):
    self.status = status
    self.response = response
    self.message = message

  def read(self, iprot):
    if iprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and isinstance(iprot.trans, TTransport.CReadableTransport) and self.thrift_spec is not None and fastbinary is not None:
      fastbinary.decode_binary(self, iprot.trans, (self.__class__, self.thrift_spec))
      return
    iprot.readStructBegin()
    while True:
      (fname, ftype, fid) = iprot.readFieldBegin()
      if ftype == TType.STOP:
        break
      if fid == 1:
        if ftype == TType.STRING:
          self.status = iprot.readString();
        else:
          iprot.skip(ftype)
      elif fid == 2:
        if ftype == TType.STRUCT:
          self.response = GamestateResponse()
          self.response.read(iprot)
        else:
          iprot.skip(ftype)
      elif fid == 3:
        if ftype == TType.STRING:
          self.message = iprot.readString();
        else:
          iprot.skip(ftype)
      else:
        iprot.skip(ftype)
      iprot.readFieldEnd()
    iprot.readStructEnd()

  def write(self, oprot):
    if oprot.__class__ == TBinaryProtocol.TBinaryProtocolAccelerated and self.thrift_spec is not None and fastbinary is not None:
      oprot.trans.write(fastbinary.encode_binary(self, (self.__class__, self.thrift_spec)))
      return
    oprot.writeStructBegin('GetMoveResponse')
    if self.status != None:
      oprot.writeFieldBegin('status', TType.STRING, 1)
      oprot.writeString(self.status)
      oprot.writeFieldEnd()
    if self.response != None:
      oprot.writeFieldBegin('response', TType.STRUCT, 2)
      self.response.write(oprot)
      oprot.writeFieldEnd()
    if self.message != None:
      oprot.writeFieldBegin('message', TType.STRING, 3)
      oprot.writeString(self.message)
      oprot.writeFieldEnd()
    oprot.writeFieldStop()
    oprot.writeStructEnd()

  def __repr__(self):
    L = ['%s=%r' % (key, value)
      for key, value in self.__dict__.iteritems()]
    return '%s(%s)' % (self.__class__.__name__, ', '.join(L))

  def __eq__(self, other):
    return isinstance(other, self.__class__) and self.__dict__ == other.__dict__

  def __ne__(self, other):
    return not (self == other)

