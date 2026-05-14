const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>MKR1000 PCB Monitor</title>
    <style>
        body { 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; 
            background-color: #1a1a1a; 
            color: white; 
            display: flex; 
            flex-direction: column; 
            align-items: center; 
            justify-content: center; 
            height: 100vh; 
            margin: 0; 
            overflow: hidden;
        }
        
        h3 { margin-bottom: 15px; color: #00ff00; text-transform: uppercase; letter-spacing: 2px; }

        /* A zöld NYÁK lap */
        .pcb { 
            background-color: #004d00; 
            width: 280px; 
            height: 360px; 
            border-radius: 15px; 
            position: relative; 
            border: 4px solid #003300;
            box-shadow: 0 15px 35px rgba(0,0,0,0.7);
            padding: 20px;
            box-sizing: border-box;
        }

        /* Gombok rácsa (4x4) */
        .matrix { 
            display: grid; 
            grid-template-columns: repeat(4, 1fr); 
            gap: 12px; 
            margin-top: 10px;
        }

        /* Takt kapcsoló alapja */
        .btn-base { 
            width: 50px; 
            height: 50px; 
            background: #b0b0b0; 
            border-radius: 4px; 
            display: flex; 
            align-items: center; 
            justify-content: center; 
            position: relative;
            box-shadow: inset 0 0 5px rgba(0,0,0,0.5);
        }

        /* A fekete nyomógomb feje */
        .btn-cap { 
            width: 30px; 
            height: 30px; 
            background: #111; 
            border-radius: 50%; 
            border: 2px solid #333; 
            transition: all 0.15s ease-in-out;
        }

        /* Aktív állapot (ha a placeholderbe bekerül az 'active') */
        .active .btn-cap { 
            background: #ff9800; 
            border-color: #fff; 
            box-shadow: 0 0 20px #ff9800; 
            transform: scale(0.9);
        }
        .active .btn-base { 
            background: #ffffff; 
        }

        /* Feliratok a csatlakozóhoz */
        .labels { 
            position: absolute; 
            bottom: 40px; 
            left: 50%; 
            transform: translateX(-50%); 
            font-size: 10px; 
            color: #7ab37a; 
            font-weight: bold;
            white-space: nowrap; 
        }

        /* Aranyozott tűsor alul */
        .header-pins { 
            position: absolute; 
            bottom: 15px; 
            left: 50%; 
            transform: translateX(-50%); 
            display: flex; 
            gap: 5px; 
        }

        .pin { 
            width: 10px; 
            height: 18px; 
            background: linear-gradient(to bottom, #ffd700, #b8860b); 
            border: 1px solid #8a6d00; 
            border-radius: 1px;
        }
        
        .info { margin-top: 20px; font-size: 12px; color: #666; }
    </style>
</head>
<body>
    <h3>Matrix Monitor</h3>
    <div class="pcb">
        <div class="matrix">
            <!-- 1. sor -->
            <div class="btn-base @C1@"><div class="btn-cap"></div></div>
            <div class="btn-base @C2@"><div class="btn-cap"></div></div>
            <div class="btn-base @C3@"><div class="btn-cap"></div></div>
            <div class="btn-base @C4@"><div class="btn-cap"></div></div>
            <!-- 2. sor -->
            <div class="btn-base @C5@"><div class="btn-cap"></div></div>
            <div class="btn-base @C6@"><div class="btn-cap"></div></div>
            <div class="btn-base @C7@"><div class="btn-cap"></div></div>
            <div class="btn-base @C8@"><div class="btn-cap"></div></div>
            <!-- 3. sor -->
            <div class="btn-base @C9@"><div class="btn-cap"></div></div>
            <div class="btn-base @C10@"><div class="btn-cap"></div></div>
            <div class="btn-base @C11@"><div class="btn-cap"></div></div>
            <div class="btn-base @C12@"><div class="btn-cap"></div></div>
            <!-- 4. sor -->
            <div class="btn-base @C13@"><div class="btn-cap"></div></div>
            <div class="btn-base @C14@"><div class="btn-cap"></div></div>
            <div class="btn-base @C15@"><div class="btn-cap"></div></div>
            <div class="btn-base @C16@"><div class="btn-cap"></div></div>
        </div>
        
        <div class="labels">R1 R2 R3 R4 C1 C2 C3 C4</div>
        <div class="header-pins">
            <div class="pin"></div><div class="pin"></div><div class="pin"></div><div class="pin"></div>
            <div class="pin"></div><div class="pin"></div><div class="pin"></div><div class="pin"></div>
        </div>
    </div>
    <div class="info">Auto-refresh aktív (1s)</div>
</body>
</html>
)=====";