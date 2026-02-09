# Main GNSS workflow
```mermaid
graph TD
    Start([Start]) --> InitModem[nrf_modem_lib_init]
    InitModem --> ConfigRef[อ่านพิกัดอ้างอิงจาก Config]
    ConfigRef --> ModemInit[modem_init:
    ต่อ LTE/รอเวลาจากเครือข่าย]
    ModemInit --> SampleInit[sample_init:
    สร้าง Work Queue และ 
    Init Assistance]
    SampleInit --> GNSSInit[gnss_init_and_start: ตั้งค่า Handler/NMEA/Mode]
    GNSSInit --> Loop{{Main Loop: k_poll}}

    Loop -->|PVT Event| HandlePVT[
        **จัดการข้อมูล PVT**
        ตำแหน่ง/ความเร็ว/เวลา
        ]
    Loop -->|NMEA Event| HandleNMEA[
        **จัดการข้อมูล NMEA**
        แสดงผล String]

    HandlePVT --> PrintFix{Fix Valid?}
    PrintFix -->|Yes| ShowData[แสดงพิกัดและคำนวณระยะห่าง]
    PrintFix -->|No| ShowWait[แสดงเวลาที่รอ]
    
    ShowData --> Loop
    ShowWait --> Loop
    HandleNMEA --> Loop
```

# A-GNSS
```mermaid
sequenceDiagram
    participant Modem as nRF Modem (Hardware)
    participant Handler as gnss_event_handler
    participant WQ as gnss_work_q (Thread)
    participant Asst as Assistance Library

    Modem->>Handler: NRF_MODEM_GNSS_EVT_AGNSS_REQ
    Handler->>WQ: k_work_submit(agnss_data_get_work)
    
    Note over WQ: agnss_data_get_work_fn
    
    OPT ถ้าเป็น LTE_ON_DEMAND
        WQ->>Modem: lte_connect()
    END
    
    WQ->>Asst: assistance_request(&last_agnss)
    Asst-->>WQ: ส่งข้อมูลช่วยเหลือเข้าสู่ Modem
    
    OPT ถ้าเป็น LTE_ON_DEMAND
        WQ->>Modem: lte_disconnect()
    END
    
    Note over WQ: ปลดธง requesting_assistance = false
```
**ส่วนประกอบ,หน้าที่หลัก**
- pvt_data_sem,สัญญาณบอกว่ามีข้อมูลตำแหน่งใหม่ (PVT) เข้ามาแล้ว
- nmea_queue,คิวเก็บข้อมูล NMEA String ที่ได้รับจาก Modem
- gnss_work_q,Thread แยกสำหรับจัดการงานที่ใช้เวลานาน (เช่น ขอ A-GNSS หรือ TTFF Test)
- distance_calculate,ใช้สูตร Haversine เพื่อหาระยะทางจากจุดอ้างอิง


