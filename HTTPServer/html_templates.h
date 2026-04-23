#pragma once

namespace HtmlTemplates {
    // Шаблон с плейсхолдерами {lang}, {title}, {browser_color} и т.д.
    const std::string HTML_TEMPLATE = R"(
<!DOCTYPE html>
<html lang='{lang}'>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>{title}</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        }
        
        body {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }
        
        .container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
            padding: 40px;
            width: 100%;
            max-width: 600px;
            animation: fadeIn 0.8s ease-out;
        }
        
        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(20px); }
            to { opacity: 1; transform: translateY(0); }
        }
        
        .header {
            text-align: center;
            margin-bottom: 30px;
        }
        
        h1 {
            color: #333;
            margin-bottom: 10px;
            font-size: 2.5rem;
        }
        
        .subtitle {
            color: #666;
            font-size: 1.1rem;
        }
        
        .info-card {
            background: #f8f9fa;
            border-radius: 15px;
            padding: 25px;
            margin-bottom: 25px;
            border-left: 5px solid {browser_color};
        }
        
        .info-item {
            display: flex;
            justify-content: space-between;
            margin-bottom: 15px;
            padding-bottom: 15px;
            border-bottom: 1px solid #e9ecef;
        }
        
        .info-item:last-child {
            margin-bottom: 0;
            padding-bottom: 0;
            border-bottom: none;
        }
        
        .label {
            font-weight: 600;
            color: #495057;
        }
        
        .value {
            font-weight: 700;
            color: {browser_color};
        }
        
        .device-emoji {
            font-size: 1.5rem;
        }
        
        .footer {
            text-align: center;
            margin-top: 30px;
            color: #6c757d;
            font-size: 0.9rem;
        }
        
        .request-number {
            display: inline-block;
            background: {browser_color};
            color: white;
            padding: 5px 15px;
            border-radius: 20px;
            font-weight: bold;
            margin-top: 10px;
        }
        
        .server-info {
            background: #e3f2fd;
            border-radius: 10px;
            padding: 15px;
            margin-top: 20px;
            font-family: monospace;
            font-size: 0.9rem;
        }
    </style>
</head>
<body>
    <div class='container'>
        <div class='header'>
            <h1>🚀 {title}</h1>
            <p class='subtitle'>{welcome}</p>
        </div>
        
        <div class='info-card'>
            <div class='info-item'>
                <span class='label'>{browser_label}</span>
                <span class='value'>{browser}</span>
            </div>
            
            <div class='info-item'>
                <span class='label'>{os_label}</span>
                <span class='value'>{os}</span>
            </div>
            
            <div class='info-item'>
                <span class='label'>{device_label}</span>
                <span>
                    <span class='device-emoji'>{device_emoji}</span>
                    <span class='value'>{device}</span>
                </span>
            </div>
        </div>
        
        <div class='footer'>
            <div class='request-number'>{request_label} #{request_number}</div>
            <p style='margin-top: 20px;'>This page was served by a C++ HTTP server</p>
            
            <div class='server-info'>
                Server info: C++ · Windows · Port {port_number} · {thread_info}
            </div>
        </div>
    </div>
</body>
</html>
)";
}