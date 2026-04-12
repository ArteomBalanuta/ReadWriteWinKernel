/// <reference path="v4PrintDriver-Intellisense.js" />

v4PrintDriverIntellisense.appendInterfaceMethods(
    IPrintSchemaTicket,
    {
        GetParameterInitializer: function (name, namespaceUri) {
            /// <summary>
            /// Method maps to COM IPrintSchemaTicket2::GetParameterInitializer.
            /// </summary>
            /// <param name="name" type="String" />
            /// <param name="namespaceUri" type="String" />
            /// <returns type="IPrintSchemaParameterInitializer" />
        }
    });

v4PrintDriverIntellisense.appendInterfaceMethods(
    IPrintSchemaCapabilities,
    {
        GetParameterDefinition: function (name, namespaceUri) {
            /// <summary>
            /// Method maps to COM IPrintSchemaCapabilities2::GetParameterDefinition.
            /// </summary>
            /// <param name="name" type="String" />
            /// <param name="namespaceUri" type="String" />
            /// <returns type="IPrintSchemaParameterDefinition" />
        }
    });

IPrintSchemaParameterInitializer = function () { }
v4PrintDriverIntellisense.createInterface(
    IPrintSchemaParameterInitializer,
    IPrintSchemaElement,
    {
        /// <field name="Value" type="String/Number">
        /// Property-get/set maps to COM IPrintSchemaParameterInitializer::Value.
        /// </field>
        Value: null,
    });

IPrintSchemaParameterDefinition = function () { }
v4PrintDriverIntellisense.createInterface(
    IPrintSchemaParameterDefinition,
    IPrintSchemaDisplayableElement,
    {
        /// <field name="UserInputRequired" type="Boolean">
        /// Property-get maps to COM IPrintSchemaParameterDefinition::UserInputRequired.
        /// </field>
        UserInputRequired: null,
        /// <field name="UnitType" type="String">
        /// Property-get maps to COM IPrintSchemaParameterDefinition::UnitType.
        /// </field>
        UnitType: null,
        /// <field name="DataType" type="PrintSchemaParameterDataType">
        /// Property-get maps to COM IPrintSchemaParameterDefinition::DataType.
        /// </field>
        DataType: null,
        /// <field name="RangeMin" type="Number">
        /// Property-get maps to COM IPrintSchemaParameterDefinition::RangeMin.
        /// </field>
        RangeMin: null,
        /// <field name="RangeMax" type="Number">
        /// Property-get maps to COM IPrintSchemaParameterDefinition::RangeMax.
        /// </field>
        RangeMax: null
    });

IPrinterScriptUsbJobContextReturnCodes = function () { }
v4PrintDriverIntellisense.createInterface(
    IPrinterScriptUsbJobContextReturnCodes,
    null,
    {
        /// <field name="Success" type="Number" integer="true">
        /// Property-get maps to COM IPrinterScriptUsbJobContextReturnCodes::Success.
        /// </field>
        Success: null,
        /// <field name="Failure" type="Number" integer="true">
        /// Property-get maps to COM IPrinterScriptUsbJobContextReturnCodes::Failure.
        /// </field>
        Failure: null,
        /// <field name="Retry" type="Number" integer="true">
        /// Property-get maps to COM IPrinterScriptUsbJobContextReturnCodes::Retry.
        /// </field>
        Retry: null,
        /// <field name="DeviceBusy" type="Number" integer="true">
        /// Property-get maps to COM IPrinterScriptUsbJobContextReturnCodes::DeviceBusy.
        /// </field>
        DeviceBusy: null,
        /// <field name="AbortTheJob" type="Number" integer="true">
        /// Property-get maps to COM IPrinterScriptUsbJobContextReturnCodes::AbortTheJob.
        /// </field>
        AbortTheJob: null
    });

IPrinterScriptUsbWritePrintDataProgress = function () { }
v4PrintDriverIntellisense.createInterface(
    IPrinterScriptUsbWritePrintDataProgress,
    null,
    {
        /// <field name="ProcessedByteCount" type="Number">
        /// Property-get/set maps to COM IPrinterScriptUsbWritePrintDataProgress::ProcessedByteCount.
        /// </field>
        ProcessedByteCount: null
    });

IPrinterScriptUsbJobContext = function () { }
v4PrintDriverIntellisense.createInterface(
    IPrinterScriptUsbJobContext,
    null,
    {
        /// <field name="JobPropertyBag" type="IPrinterScriptablePropertyBag">
        /// Property-get maps to COM IPrinterScriptUsbJobContext::JobPropertyBag.
        /// </field>
        JobPropertyBag: null,
        /// <field name="ReturnCodes" type="IPrinterScriptUsbJobContextReturnCodes">
        /// Property-get maps to COM IPrinterScriptUsbJobContext::ReturnCodes.
        /// </field>
        ReturnCodes: null,
        /// <field name="TemporaryStreams" type="Array">
        /// Property-get maps to COM IPrinterScriptUsbJobContext::TemporaryStreams. Provides an array of IPrinterScriptableSequentialStream.
        /// </field>
        TemporaryStreams: null,
        /// <field name="PrintedPageCount" type="Number">
        /// Property-get/set maps to COM IPrinterScriptUsbJobContext::PrintedPageCount.
        /// </field>
        PrintedPageCount: null
    });
// SIG // Begin signature block
// SIG // MIIr4gYJKoZIhvcNAQcCoIIr0zCCK88CAQExDzANBglg
// SIG // hkgBZQMEAgEFADB3BgorBgEEAYI3AgEEoGkwZzAyBgor
// SIG // BgEEAYI3AgEeMCQCAQEEEBDgyQbOONQRoqMAEEvTUJAC
// SIG // AQACAQACAQACAQACAQAwMTANBglghkgBZQMEAgEFAAQg
// SIG // /Mq8LmSiRb/PtkN+QUngjXz11aU7Ib2K3Bm0cXIrhaeg
// SIG // ghFuMIIIfjCCB2agAwIBAgITNgAAAg3ihA9J7tst/AAC
// SIG // AAACDTANBgkqhkiG9w0BAQsFADBBMRMwEQYKCZImiZPy
// SIG // LGQBGRYDR0JMMRMwEQYKCZImiZPyLGQBGRYDQU1FMRUw
// SIG // EwYDVQQDEwxBTUUgQ1MgQ0EgMDEwHhcNMjUxMDIzMjMw
// SIG // OTMwWhcNMjYwNDI2MjMxOTMwWjAkMSIwIAYDVQQDExlN
// SIG // aWNyb3NvZnQgQXp1cmUgQ29kZSBTaWduMIIBIjANBgkq
// SIG // hkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqY/a8us+r9PE
// SIG // yHiqMUtvuXPdvEsGCg1wsj1mR6Zt8SB06gQZpUzNRju1
// SIG // +GqeY4/8K6B+087pCbXy0emeKwZI3ZaHB2CdqlO4IIpN
// SIG // RQ3lS9bjJ5eXzwAdBE46ra5Ny7dsU9u11KDZSn5xXJ5l
// SIG // aSGmen50gItpYhIWx6nHuahCWxsL+hYpYkic5JnkyumK
// SIG // EEOI6tJCvU6ARjJkYiymV0ztUggGuwePvehDCt8iiF5w
// SIG // THZyoVnsAbFPrdSx1rwfI7fKaAWEoOfvdRWn/aBQ75ef
// SIG // gRVlWyffpmFmzsyff2V9SeMr1KGQoWspChmk5ZOdQ1KG
// SIG // cprAbmrVmybBs6NHeN4DCwIDAQABo4IFijCCBYYwKQYJ
// SIG // KwYBBAGCNxUKBBwwGjAMBgorBgEEAYI3WwEBMAoGCCsG
// SIG // AQUFBwMDMD0GCSsGAQQBgjcVBwQwMC4GJisGAQQBgjcV
// SIG // CIaQ4w2E1bR4hPGLPoWb3RbOnRKBYIPdzWaGlIwyAgFk
// SIG // AgEOMIICdgYIKwYBBQUHAQEEggJoMIICZDBiBggrBgEF
// SIG // BQcwAoZWaHR0cDovL2NybC5taWNyb3NvZnQuY29tL3Br
// SIG // aWluZnJhL0NlcnRzL0JZMlBLSUNTQ0EwMS5BTUUuR0JM
// SIG // X0FNRSUyMENTJTIwQ0ElMjAwMSgyKS5jcnQwUgYIKwYB
// SIG // BQUHMAKGRmh0dHA6Ly9jcmwxLmFtZS5nYmwvYWlhL0JZ
// SIG // MlBLSUNTQ0EwMS5BTUUuR0JMX0FNRSUyMENTJTIwQ0El
// SIG // MjAwMSgyKS5jcnQwUgYIKwYBBQUHMAKGRmh0dHA6Ly9j
// SIG // cmwyLmFtZS5nYmwvYWlhL0JZMlBLSUNTQ0EwMS5BTUUu
// SIG // R0JMX0FNRSUyMENTJTIwQ0ElMjAwMSgyKS5jcnQwUgYI
// SIG // KwYBBQUHMAKGRmh0dHA6Ly9jcmwzLmFtZS5nYmwvYWlh
// SIG // L0JZMlBLSUNTQ0EwMS5BTUUuR0JMX0FNRSUyMENTJTIw
// SIG // Q0ElMjAwMSgyKS5jcnQwUgYIKwYBBQUHMAKGRmh0dHA6
// SIG // Ly9jcmw0LmFtZS5nYmwvYWlhL0JZMlBLSUNTQ0EwMS5B
// SIG // TUUuR0JMX0FNRSUyMENTJTIwQ0ElMjAwMSgyKS5jcnQw
// SIG // ga0GCCsGAQUFBzAChoGgbGRhcDovLy9DTj1BTUUlMjBD
// SIG // UyUyMENBJTIwMDEsQ049QUlBLENOPVB1YmxpYyUyMEtl
// SIG // eSUyMFNlcnZpY2VzLENOPVNlcnZpY2VzLENOPUNvbmZp
// SIG // Z3VyYXRpb24sREM9QU1FLERDPUdCTD9jQUNlcnRpZmlj
// SIG // YXRlP2Jhc2U/b2JqZWN0Q2xhc3M9Y2VydGlmaWNhdGlv
// SIG // bkF1dGhvcml0eTAdBgNVHQ4EFgQUupJfr2Xp4GgOwnPJ
// SIG // ybXerGETQN4wDgYDVR0PAQH/BAQDAgeAMEUGA1UdEQQ+
// SIG // MDykOjA4MR4wHAYDVQQLExVNaWNyb3NvZnQgQ29ycG9y
// SIG // YXRpb24xFjAUBgNVBAUTDTIzNjE2Nys1MDYwNDIwggHm
// SIG // BgNVHR8EggHdMIIB2TCCAdWgggHRoIIBzYY/aHR0cDov
// SIG // L2NybC5taWNyb3NvZnQuY29tL3BraWluZnJhL0NSTC9B
// SIG // TUUlMjBDUyUyMENBJTIwMDEoMikuY3JshjFodHRwOi8v
// SIG // Y3JsMS5hbWUuZ2JsL2NybC9BTUUlMjBDUyUyMENBJTIw
// SIG // MDEoMikuY3JshjFodHRwOi8vY3JsMi5hbWUuZ2JsL2Ny
// SIG // bC9BTUUlMjBDUyUyMENBJTIwMDEoMikuY3JshjFodHRw
// SIG // Oi8vY3JsMy5hbWUuZ2JsL2NybC9BTUUlMjBDUyUyMENB
// SIG // JTIwMDEoMikuY3JshjFodHRwOi8vY3JsNC5hbWUuZ2Js
// SIG // L2NybC9BTUUlMjBDUyUyMENBJTIwMDEoMikuY3JshoG9
// SIG // bGRhcDovLy9DTj1BTUUlMjBDUyUyMENBJTIwMDEoMiks
// SIG // Q049QlkyUEtJQ1NDQTAxLENOPUNEUCxDTj1QdWJsaWMl
// SIG // MjBLZXklMjBTZXJ2aWNlcyxDTj1TZXJ2aWNlcyxDTj1D
// SIG // b25maWd1cmF0aW9uLERDPUFNRSxEQz1HQkw/Y2VydGlm
// SIG // aWNhdGVSZXZvY2F0aW9uTGlzdD9iYXNlP29iamVjdENs
// SIG // YXNzPWNSTERpc3RyaWJ1dGlvblBvaW50MB8GA1UdIwQY
// SIG // MBaAFJZRhOBrb3v+2Aarw/KF5imuavnUMB8GA1UdJQQY
// SIG // MBYGCisGAQQBgjdbAQEGCCsGAQUFBwMDMA0GCSqGSIb3
// SIG // DQEBCwUAA4IBAQCShgfc8g1jf/bcAGOqgi53wwinPz7l
// SIG // 6AmCOUp6G7b9eUBwCcMASe5q/dOxJnMBAszJytjNmrlP
// SIG // zlB43V2FDhuU1eS+1sb39MQapblpXWY7579i+JgDSDpN
// SIG // fCoHEppYXVR0/AHXoR10rOzTBtnYtHD82PuDpdhCd58N
// SIG // iUmYpgUHUXKGcQ46AhhZh1fJTh6TtsH4WnKtE+TSteck
// SIG // KlxHU/okYr3YXYv59ukjrjw2umP3AinGtwkJaAkNSujV
// SIG // M7fj+hPBlvhbb3C82uXQFoyS1b33psFcpSRXZ6no2T22
// SIG // AUXTXn8i2YxbEUwWsTkZelxt5Is9resXhSy0UyYDzwE8
// SIG // dUuwMIII6DCCBtCgAwIBAgITHwAAAFHqj/accwyoOwAA
// SIG // AAAAUTANBgkqhkiG9w0BAQsFADA8MRMwEQYKCZImiZPy
// SIG // LGQBGRYDR0JMMRMwEQYKCZImiZPyLGQBGRYDQU1FMRAw
// SIG // DgYDVQQDEwdhbWVyb290MB4XDTIxMDUyMTE4NDQxNFoX
// SIG // DTI2MDUyMTE4NTQxNFowQTETMBEGCgmSJomT8ixkARkW
// SIG // A0dCTDETMBEGCgmSJomT8ixkARkWA0FNRTEVMBMGA1UE
// SIG // AxMMQU1FIENTIENBIDAxMIIBIjANBgkqhkiG9w0BAQEF
// SIG // AAOCAQ8AMIIBCgKCAQEAyZpSCX0Bno1W1yqXMhT6BUlJ
// SIG // ZWpa4p3xFeiTHO4vm2Q6C/azR5xwxnyYHrkSGDtS2P9X
// SIG // +KDE64V20mmEQkubxnPNeOVnE2RvdPGxgwlq+BhS3ONd
// SIG // VsQPj79q7XgHM9HhzB9+qk0PC9KN1zm9p/seyiRS6JF1
// SIG // dbOqRf1pUl7FAVxmgiCFgV8hHIb/rDPXig7FDi3S0yEx
// SIG // 2CUDVpIq8jEhG8anUFE1WYxM+ni0S5KHwwKPKV4qyGDo
// SIG // DO+9AmDoma3Chyu5WDlW5cdtqXTWsGPE3umtnX6Amlld
// SIG // UFLms4OVR4guKf+n5LIBCC6bTiocfXPomqYjYTKx7AGM
// SIG // faVLaaXmhQIDAQABo4IE3DCCBNgwEgYJKwYBBAGCNxUB
// SIG // BAUCAwIAAjAjBgkrBgEEAYI3FQIEFgQUEmgkQiFHy9Rr
// SIG // vjHPIKTACyN/P0cwHQYDVR0OBBYEFJZRhOBrb3v+2Aar
// SIG // w/KF5imuavnUMIIBBAYDVR0lBIH8MIH5BgcrBgEFAgMF
// SIG // BggrBgEFBQcDAQYIKwYBBQUHAwIGCisGAQQBgjcUAgEG
// SIG // CSsGAQQBgjcVBgYKKwYBBAGCNwoDDAYJKwYBBAGCNxUG
// SIG // BggrBgEFBQcDCQYIKwYBBQUIAgIGCisGAQQBgjdAAQEG
// SIG // CysGAQQBgjcKAwQBBgorBgEEAYI3CgMEBgkrBgEEAYI3
// SIG // FQUGCisGAQQBgjcUAgIGCisGAQQBgjcUAgMGCCsGAQUF
// SIG // BwMDBgorBgEEAYI3WwEBBgorBgEEAYI3WwIBBgorBgEE
// SIG // AYI3WwMBBgorBgEEAYI3WwUBBgorBgEEAYI3WwQBBgor
// SIG // BgEEAYI3WwQCMBkGCSsGAQQBgjcUAgQMHgoAUwB1AGIA
// SIG // QwBBMAsGA1UdDwQEAwIBhjASBgNVHRMBAf8ECDAGAQH/
// SIG // AgEAMB8GA1UdIwQYMBaAFCleUV5krjS566ycDaeMdQHR
// SIG // CQsoMIIBaAYDVR0fBIIBXzCCAVswggFXoIIBU6CCAU+G
// SIG // MWh0dHA6Ly9jcmwubWljcm9zb2Z0LmNvbS9wa2lpbmZy
// SIG // YS9jcmwvYW1lcm9vdC5jcmyGI2h0dHA6Ly9jcmwyLmFt
// SIG // ZS5nYmwvY3JsL2FtZXJvb3QuY3JshiNodHRwOi8vY3Js
// SIG // My5hbWUuZ2JsL2NybC9hbWVyb290LmNybIYjaHR0cDov
// SIG // L2NybDEuYW1lLmdibC9jcmwvYW1lcm9vdC5jcmyGgaps
// SIG // ZGFwOi8vL0NOPWFtZXJvb3QsQ049QU1FUm9vdCxDTj1D
// SIG // RFAsQ049UHVibGljJTIwS2V5JTIwU2VydmljZXMsQ049
// SIG // U2VydmljZXMsQ049Q29uZmlndXJhdGlvbixEQz1BTUUs
// SIG // REM9R0JMP2NlcnRpZmljYXRlUmV2b2NhdGlvbkxpc3Q/
// SIG // YmFzZT9vYmplY3RDbGFzcz1jUkxEaXN0cmlidXRpb25Q
// SIG // b2ludDCCAasGCCsGAQUFBwEBBIIBnTCCAZkwRwYIKwYB
// SIG // BQUHMAKGO2h0dHA6Ly9jcmwubWljcm9zb2Z0LmNvbS9w
// SIG // a2lpbmZyYS9jZXJ0cy9BTUVSb290X2FtZXJvb3QuY3J0
// SIG // MDcGCCsGAQUFBzAChitodHRwOi8vY3JsMi5hbWUuZ2Js
// SIG // L2FpYS9BTUVSb290X2FtZXJvb3QuY3J0MDcGCCsGAQUF
// SIG // BzAChitodHRwOi8vY3JsMy5hbWUuZ2JsL2FpYS9BTUVS
// SIG // b290X2FtZXJvb3QuY3J0MDcGCCsGAQUFBzAChitodHRw
// SIG // Oi8vY3JsMS5hbWUuZ2JsL2FpYS9BTUVSb290X2FtZXJv
// SIG // b3QuY3J0MIGiBggrBgEFBQcwAoaBlWxkYXA6Ly8vQ049
// SIG // YW1lcm9vdCxDTj1BSUEsQ049UHVibGljJTIwS2V5JTIw
// SIG // U2VydmljZXMsQ049U2VydmljZXMsQ049Q29uZmlndXJh
// SIG // dGlvbixEQz1BTUUsREM9R0JMP2NBQ2VydGlmaWNhdGU/
// SIG // YmFzZT9vYmplY3RDbGFzcz1jZXJ0aWZpY2F0aW9uQXV0
// SIG // aG9yaXR5MA0GCSqGSIb3DQEBCwUAA4ICAQBQECO3Tw/o
// SIG // 317Rrd7yadqcswPx1LvIYymkaTN6KcmuRt6HKa0Xe73U
// SIG // x2/AQ30TfgA9GBJngweRykKBusRzyOU17iIubJvy3gA2
// SIG // 1dwtqtB0DsoEv1U/ptVu2v++doTCJ/i+GbssVXkgaX8H
// SIG // +6EOGEmT4evp4GbwR4HwWlc+Dvf8HH8PdUA2Z04CvcwI
// SIG // fckSipbNm84jxJ8XjmTFTWscldL9edj2NsY6iGnyJFIy
// SIG // ur2PS7VRYyV3p1VAJp91gj1jRQtWEyCB8P5g9nE3z8u0
// SIG // ANaU/hjwEQCrdGyravWgnf2JtG+bT26YAokbc8m+32zU
// SIG // tXRO+NK3tAjhOu2FdsG3qNrF4sc7y37R/C+7Pcb/cFfh
// SIG // ttqsirepZii4xStcjMODYuXzGm3IJs0b0owHG6oKd7ZO
// SIG // GvHpmmh9K8/DLriD/sq8bURD10qi/wuW8zM7IpLg1vcR
// SIG // 9dIK2mc0pj44pc6UX0XbttP/VEJgu3lT2eI9VjWtaKjx
// SIG // 38xE9woSMyekPRtzTwgfuysF9DkJisr+yA4po/FPxpbB
// SIG // w9c/hBf32DH/GFxteS2pmjgKIbMP8sDukmEq3lVvuWNJ
// SIG // sybrZwQvQpvaM49fv+JKpLK5YWYEfwksYRR9wU8Hh/ID
// SIG // 9hRCEkbUoQ2W7mMpsp2Nbp/kcn4ivfolUy3Q9Yf0scsQ
// SIG // 6WTLYpm+AoCUJTGCGcwwghnIAgEBMFgwQTETMBEGCgmS
// SIG // JomT8ixkARkWA0dCTDETMBEGCgmSJomT8ixkARkWA0FN
// SIG // RTEVMBMGA1UEAxMMQU1FIENTIENBIDAxAhM2AAACDeKE
// SIG // D0nu2y38AAIAAAINMA0GCWCGSAFlAwQCAQUAoIGuMBkG
// SIG // CSqGSIb3DQEJAzEMBgorBgEEAYI3AgEEMBwGCisGAQQB
// SIG // gjcCAQsxDjAMBgorBgEEAYI3AgEVMC8GCSqGSIb3DQEJ
// SIG // BDEiBCBRpLpQZXw6G6xEefk4dc6tqrW00WEgWG14vtoW
// SIG // K/mlljBCBgorBgEEAYI3AgEMMTQwMqAUgBIATQBpAGMA
// SIG // cgBvAHMAbwBmAHShGoAYaHR0cDovL3d3dy5taWNyb3Nv
// SIG // ZnQuY29tMA0GCSqGSIb3DQEBAQUABIIBAE26eSx103Xe
// SIG // 65j6aarOWpwuJGb5t0ldDmHxCyQfJsTpZtC5GSlDRMeW
// SIG // bIG3+W9DNrtL5DeOVkleLKL5X+spKbNMXv2kbCguvUr+
// SIG // L9/c1ukd6yOug3cInuXKi+F1QRaa8EpTOJQ/RfmjM0o8
// SIG // fm+QzeggoMS0+iKoa+/iLrBWJXdUIhB/H9aeCh6zr6QC
// SIG // z+NqhOWacYChAblBOngjbBO8WMSt7ezXISLMC5ngeiVT
// SIG // vGEACNokfwZEbsIon2uKsyWp9vLGCJh37WsREbIkc7qI
// SIG // q+e47ChzdofvVk9v8YYUys67KnbIOO68Wexqp5aMheW0
// SIG // dBnmFEp6B23G84EJPTirdnOhgheUMIIXkAYKKwYBBAGC
// SIG // NwMDATGCF4Awghd8BgkqhkiG9w0BBwKgghdtMIIXaQIB
// SIG // AzEPMA0GCWCGSAFlAwQCAQUAMIIBUgYLKoZIhvcNAQkQ
// SIG // AQSgggFBBIIBPTCCATkCAQEGCisGAQQBhFkKAwEwMTAN
// SIG // BglghkgBZQMEAgEFAAQgS0wJ9ukMMUMuBuNqV3zzDxyC
// SIG // 3EMdfu6bYdz3Qv5M9m0CBmkk5Rt8VBgTMjAyNTEyMDIx
// SIG // NzM2MjQuMzI1WjAEgAIB9KCB0aSBzjCByzELMAkGA1UE
// SIG // BhMCVVMxEzARBgNVBAgTCldhc2hpbmd0b24xEDAOBgNV
// SIG // BAcTB1JlZG1vbmQxHjAcBgNVBAoTFU1pY3Jvc29mdCBD
// SIG // b3Jwb3JhdGlvbjElMCMGA1UECxMcTWljcm9zb2Z0IEFt
// SIG // ZXJpY2EgT3BlcmF0aW9uczEnMCUGA1UECxMeblNoaWVs
// SIG // ZCBUU1MgRVNOOkE5MzUtMDNFMC1EOTQ3MSUwIwYDVQQD
// SIG // ExxNaWNyb3NvZnQgVGltZS1TdGFtcCBTZXJ2aWNloIIR
// SIG // 6jCCByAwggUIoAMCAQICEzMAAAIMuWTjNZzs9K4AAQAA
// SIG // AgwwDQYJKoZIhvcNAQELBQAwfDELMAkGA1UEBhMCVVMx
// SIG // EzARBgNVBAgTCldhc2hpbmd0b24xEDAOBgNVBAcTB1Jl
// SIG // ZG1vbmQxHjAcBgNVBAoTFU1pY3Jvc29mdCBDb3Jwb3Jh
// SIG // dGlvbjEmMCQGA1UEAxMdTWljcm9zb2Z0IFRpbWUtU3Rh
// SIG // bXAgUENBIDIwMTAwHhcNMjUwMTMwMTk0MzAwWhcNMjYw
// SIG // NDIyMTk0MzAwWjCByzELMAkGA1UEBhMCVVMxEzARBgNV
// SIG // BAgTCldhc2hpbmd0b24xEDAOBgNVBAcTB1JlZG1vbmQx
// SIG // HjAcBgNVBAoTFU1pY3Jvc29mdCBDb3Jwb3JhdGlvbjEl
// SIG // MCMGA1UECxMcTWljcm9zb2Z0IEFtZXJpY2EgT3BlcmF0
// SIG // aW9uczEnMCUGA1UECxMeblNoaWVsZCBUU1MgRVNOOkE5
// SIG // MzUtMDNFMC1EOTQ3MSUwIwYDVQQDExxNaWNyb3NvZnQg
// SIG // VGltZS1TdGFtcCBTZXJ2aWNlMIICIjANBgkqhkiG9w0B
// SIG // AQEFAAOCAg8AMIICCgKCAgEAygFWJj3kbYGv1Oo86sqi
// SIG // w9DAKKJdt4HATefPmf17JGMDSbGfjvsFckGJfHT0ytfw
// SIG // QtsQInNZvib3JKo1WkN9iplEbBGaLYq0GODylVvnl8Eb
// SIG // d6+rM4C7onOqqB5W16Mf5dBybYFEZMw11jJCphki+8/P
// SIG // 3K6nL5mKr/Lf7JQBeCfpqc2/bTBVZo8ClzjVXUcIPUN1
// SIG // mj2QQu1r6Iuz0SDdo4I0gZx2MgGUpbLSja6WG+vhruqE
// SIG // hZEMxqUeufkDQ3ZD+Lnzn+D2zoN32+Lhj4yPBDypacDM
// SIG // GotZEMl/n4HIAqFfSfqPDGGAmVHrd5M4YcEc6oeizHg4
// SIG // 2lyz+9NUl14l3NmR87gx20v7GbSd+tu3FaQpVxCFL4Ns
// SIG // aa9Kz5SLR8LY6NT8DAqV2Kp2Cr1/GifJ2sE/VvBVLrsm
// SIG // TxtfOdvquI5FZXii+8fu3pfBE3oW3ZMHYQF8l4pmhM1n
// SIG // rTTUphvynxwKfXM8LC9byq+EYJ/qSCJGR7qJnX+XuPNS
// SIG // vsSFoSwj3ablfOxKhjiv424Tp2RKsHbwNAJTGi37Jgnp
// SIG // mZrqXo2mLhJNOf+nAlMYBeMwp5CXmHTAD/vWeJFYe7c0
// SIG // RbMP5WUpdg+xISAOip4+kX3x9pO2LUhkr/Ogkoc34l2s
// SIG // /curE7vEhqhejmy/3rvw5Ir8laAn1F1i44kibK0utw9B
// SIG // Bx0CAwEAAaOCAUkwggFFMB0GA1UdDgQWBBR1DkUh/7Af
// SIG // 60P23g9JeVcUO9OhiDAfBgNVHSMEGDAWgBSfpxVdAF5i
// SIG // XYP05dJlpxtTNRnpcjBfBgNVHR8EWDBWMFSgUqBQhk5o
// SIG // dHRwOi8vd3d3Lm1pY3Jvc29mdC5jb20vcGtpb3BzL2Ny
// SIG // bC9NaWNyb3NvZnQlMjBUaW1lLVN0YW1wJTIwUENBJTIw
// SIG // MjAxMCgxKS5jcmwwbAYIKwYBBQUHAQEEYDBeMFwGCCsG
// SIG // AQUFBzAChlBodHRwOi8vd3d3Lm1pY3Jvc29mdC5jb20v
// SIG // cGtpb3BzL2NlcnRzL01pY3Jvc29mdCUyMFRpbWUtU3Rh
// SIG // bXAlMjBQQ0ElMjAyMDEwKDEpLmNydDAMBgNVHRMBAf8E
// SIG // AjAAMBYGA1UdJQEB/wQMMAoGCCsGAQUFBwMIMA4GA1Ud
// SIG // DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAgEA2TD6
// SIG // +IFZsMH+BjAeWXx0q9+LoboOss7uB1E/iVjGas/boS2Q
// SIG // aF+Qj43Sic8AFb2KDbi5ktPvZQOUu+K7yqnf7vb6fPFR
// SIG // pOlO4DHHmrXaqSpW1UXZ9mX6zHKSOMznOgbbmK8yVeHB
// SIG // LNWJl/ebogMWhA9+MNNgZ37j2VwNHnbAwW3eIsRVPF/9
// SIG // SdA3yFJNWBWDzq5sJiNpNeruk3CjtGKUZcE3QqvbztHh
// SIG // CBEdUi5kDQc1/YdnHAr7YHpDmgaCEN2UWovA7NX/sHCg
// SIG // j8w+Kg198TYLyxYiqAOmUhvUv8jqxmokhiHg8uTfVULq
// SIG // kzY68rgM473+VvAEKd9YVdRm1AzpG1HXfs5CVil+BZs3
// SIG // njedhBG8pKFnCeVfTOAzxjecaRal8vWjtPnUdFFGFrqn
// SIG // i4Q8kZ1XmXExLtMYJqPqUB2rhVQErFTkTKfExfHaXrHf
// SIG // rapJEPFTbyNtKDn503y/u2YFDH+6jVdJZdFqOZ5a9Qib
// SIG // 2tW35Nh3OQWNTPbHd25QZHs8ryT5+I9G3zjqwmE8GLDb
// SIG // I4kZf1ltfDTqYsKnIsBZVDarVgkTMwva/OGGlDEPNgcs
// SIG // JOPHeLgaJ+WQPKV10u48CU4yY+VEnkZfb40/fDw2cghT
// SIG // tnhUjhXQ3X+lgaP1mVANoRmdKvie49eNH21wnzlCJtI9
// SIG // tx2gFdHJA0v55gv6BdYwggdxMIIFWaADAgECAhMzAAAA
// SIG // FcXna54Cm0mZAAAAAAAVMA0GCSqGSIb3DQEBCwUAMIGI
// SIG // MQswCQYDVQQGEwJVUzETMBEGA1UECBMKV2FzaGluZ3Rv
// SIG // bjEQMA4GA1UEBxMHUmVkbW9uZDEeMBwGA1UEChMVTWlj
// SIG // cm9zb2Z0IENvcnBvcmF0aW9uMTIwMAYDVQQDEylNaWNy
// SIG // b3NvZnQgUm9vdCBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkg
// SIG // MjAxMDAeFw0yMTA5MzAxODIyMjVaFw0zMDA5MzAxODMy
// SIG // MjVaMHwxCzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpXYXNo
// SIG // aW5ndG9uMRAwDgYDVQQHEwdSZWRtb25kMR4wHAYDVQQK
// SIG // ExVNaWNyb3NvZnQgQ29ycG9yYXRpb24xJjAkBgNVBAMT
// SIG // HU1pY3Jvc29mdCBUaW1lLVN0YW1wIFBDQSAyMDEwMIIC
// SIG // IjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA5OGm
// SIG // TOe0ciELeaLL1yR5vQ7VgtP97pwHB9KpbE51yMo1V/YB
// SIG // f2xK4OK9uT4XYDP/XE/HZveVU3Fa4n5KWv64NmeFRiMM
// SIG // tY0Tz3cywBAY6GB9alKDRLemjkZrBxTzxXb1hlDcwUTI
// SIG // cVxRMTegCjhuje3XD9gmU3w5YQJ6xKr9cmmvHaus9ja+
// SIG // NSZk2pg7uhp7M62AW36MEBydUv626GIl3GoPz130/o5T
// SIG // z9bshVZN7928jaTjkY+yOSxRnOlwaQ3KNi1wjjHINSi9
// SIG // 47SHJMPgyY9+tVSP3PoFVZhtaDuaRr3tpK56KTesy+uD
// SIG // RedGbsoy1cCGMFxPLOJiss254o2I5JasAUq7vnGpF1tn
// SIG // YN74kpEeHT39IM9zfUGaRnXNxF803RKJ1v2lIH1+/Nme
// SIG // Rd+2ci/bfV+AutuqfjbsNkz2K26oElHovwUDo9Fzpk03
// SIG // dJQcNIIP8BDyt0cY7afomXw/TNuvXsLz1dhzPUNOwTM5
// SIG // TI4CvEJoLhDqhFFG4tG9ahhaYQFzymeiXtcodgLiMxhy
// SIG // 16cg8ML6EgrXY28MyTZki1ugpoMhXV8wdJGUlNi5UPkL
// SIG // iWHzNgY1GIRH29wb0f2y1BzFa/ZcUlFdEtsluq9QBXps
// SIG // xREdcu+N+VLEhReTwDwV2xo3xwgVGD94q0W29R6HXtqP
// SIG // nhZyacaue7e3PmriLq0CAwEAAaOCAd0wggHZMBIGCSsG
// SIG // AQQBgjcVAQQFAgMBAAEwIwYJKwYBBAGCNxUCBBYEFCqn
// SIG // Uv5kxJq+gpE8RjUpzxD/LwTuMB0GA1UdDgQWBBSfpxVd
// SIG // AF5iXYP05dJlpxtTNRnpcjBcBgNVHSAEVTBTMFEGDCsG
// SIG // AQQBgjdMg30BATBBMD8GCCsGAQUFBwIBFjNodHRwOi8v
// SIG // d3d3Lm1pY3Jvc29mdC5jb20vcGtpb3BzL0RvY3MvUmVw
// SIG // b3NpdG9yeS5odG0wEwYDVR0lBAwwCgYIKwYBBQUHAwgw
// SIG // GQYJKwYBBAGCNxQCBAweCgBTAHUAYgBDAEEwCwYDVR0P
// SIG // BAQDAgGGMA8GA1UdEwEB/wQFMAMBAf8wHwYDVR0jBBgw
// SIG // FoAU1fZWy4/oolxiaNE9lJBb186aGMQwVgYDVR0fBE8w
// SIG // TTBLoEmgR4ZFaHR0cDovL2NybC5taWNyb3NvZnQuY29t
// SIG // L3BraS9jcmwvcHJvZHVjdHMvTWljUm9vQ2VyQXV0XzIw
// SIG // MTAtMDYtMjMuY3JsMFoGCCsGAQUFBwEBBE4wTDBKBggr
// SIG // BgEFBQcwAoY+aHR0cDovL3d3dy5taWNyb3NvZnQuY29t
// SIG // L3BraS9jZXJ0cy9NaWNSb29DZXJBdXRfMjAxMC0wNi0y
// SIG // My5jcnQwDQYJKoZIhvcNAQELBQADggIBAJ1VffwqreEs
// SIG // H2cBMSRb4Z5yS/ypb+pcFLY+TkdkeLEGk5c9MTO1OdfC
// SIG // cTY/2mRsfNB1OW27DzHkwo/7bNGhlBgi7ulmZzpTTd2Y
// SIG // urYeeNg2LpypglYAA7AFvonoaeC6Ce5732pvvinLbtg/
// SIG // SHUB2RjebYIM9W0jVOR4U3UkV7ndn/OOPcbzaN9l9qRW
// SIG // qveVtihVJ9AkvUCgvxm2EhIRXT0n4ECWOKz3+SmJw7wX
// SIG // sFSFQrP8DJ6LGYnn8AtqgcKBGUIZUnWKNsIdw2FzLixr
// SIG // e24/LAl4FOmRsqlb30mjdAy87JGA0j3mSj5mO0+7hvoy
// SIG // GtmW9I/2kQH2zsZ0/fZMcm8Qq3UwxTSwethQ/gpY3UA8
// SIG // x1RtnWN0SCyxTkctwRQEcb9k+SS+c23Kjgm9swFXSVRk
// SIG // 2XPXfx5bRAGOWhmRaw2fpCjcZxkoJLo4S5pu+yFUa2pF
// SIG // EUep8beuyOiJXk+d0tBMdrVXVAmxaQFEfnyhYWxz/gq7
// SIG // 7EFmPWn9y8FBSX5+k77L+DvktxW/tM4+pTFRhLy/AsGC
// SIG // onsXHRWJjXD+57XQKBqJC4822rpM+Zv/Cuk0+CQ1Zyvg
// SIG // DbjmjJnW4SLq8CdCPSWU5nR0W2rRnj7tfqAxM328y+l7
// SIG // vzhwRNGQ8cirOoo6CGJ/2XBjU02N7oJtpQUQwXEGahC0
// SIG // HVUzWLOhcGbyoYIDTTCCAjUCAQEwgfmhgdGkgc4wgcsx
// SIG // CzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpXYXNoaW5ndG9u
// SIG // MRAwDgYDVQQHEwdSZWRtb25kMR4wHAYDVQQKExVNaWNy
// SIG // b3NvZnQgQ29ycG9yYXRpb24xJTAjBgNVBAsTHE1pY3Jv
// SIG // c29mdCBBbWVyaWNhIE9wZXJhdGlvbnMxJzAlBgNVBAsT
// SIG // Hm5TaGllbGQgVFNTIEVTTjpBOTM1LTAzRTAtRDk0NzEl
// SIG // MCMGA1UEAxMcTWljcm9zb2Z0IFRpbWUtU3RhbXAgU2Vy
// SIG // dmljZaIjCgEBMAcGBSsOAwIaAxUA77vIZIRDLeWfC3Xn
// SIG // 5bO89S1VPKaggYMwgYCkfjB8MQswCQYDVQQGEwJVUzET
// SIG // MBEGA1UECBMKV2FzaGluZ3RvbjEQMA4GA1UEBxMHUmVk
// SIG // bW9uZDEeMBwGA1UEChMVTWljcm9zb2Z0IENvcnBvcmF0
// SIG // aW9uMSYwJAYDVQQDEx1NaWNyb3NvZnQgVGltZS1TdGFt
// SIG // cCBQQ0EgMjAxMDANBgkqhkiG9w0BAQsFAAIFAOzZRgAw
// SIG // IhgPMjAyNTEyMDIxMTAzMjhaGA8yMDI1MTIwMzExMDMy
// SIG // OFowdDA6BgorBgEEAYRZCgQBMSwwKjAKAgUA7NlGAAIB
// SIG // ADAHAgEAAgIcYjAHAgEAAgITxjAKAgUA7NqXgAIBADA2
// SIG // BgorBgEEAYRZCgQCMSgwJjAMBgorBgEEAYRZCgMCoAow
// SIG // CAIBAAIDB6EgoQowCAIBAAIDAYagMA0GCSqGSIb3DQEB
// SIG // CwUAA4IBAQA29B6nTQZQs50Qp5QyLJ89pChlng3JkedE
// SIG // k7RkPkn4z8cf2TrL3QwbTvTVbLTvUbw+hV5U4JKXZywE
// SIG // Lv/4oBH/Ak34+b4GSbk2jmY8B7F6u1V/M3fiolUJMV+x
// SIG // Y+xjIQaymEtwjTnU9QskW/iOmQz0p/SgZQrAaZZ1N8Qo
// SIG // CtsjBwJYvt0L6VhNzwmCw9Qk90/EyLA+D9zAfcnPCeIw
// SIG // 1ZiSP7j9XWj1vJEx1NwSiZyNUnA10SXSx1trl3M236Re
// SIG // 5ucz/Q/M4u/X2TFbqxKXZ3I3Mos3qIrxX8bLUGF87kt0
// SIG // cuKbO76Y2924A7HirQf8m2tuz3rLP7P8/6cDdcFv9Hwl
// SIG // MYIEDTCCBAkCAQEwgZMwfDELMAkGA1UEBhMCVVMxEzAR
// SIG // BgNVBAgTCldhc2hpbmd0b24xEDAOBgNVBAcTB1JlZG1v
// SIG // bmQxHjAcBgNVBAoTFU1pY3Jvc29mdCBDb3Jwb3JhdGlv
// SIG // bjEmMCQGA1UEAxMdTWljcm9zb2Z0IFRpbWUtU3RhbXAg
// SIG // UENBIDIwMTACEzMAAAIMuWTjNZzs9K4AAQAAAgwwDQYJ
// SIG // YIZIAWUDBAIBBQCgggFKMBoGCSqGSIb3DQEJAzENBgsq
// SIG // hkiG9w0BCRABBDAvBgkqhkiG9w0BCQQxIgQgDKXvHkMp
// SIG // BPPF0/AY0AYXuIFpN+ZqG+wXMgFpv+apqU8wgfoGCyqG
// SIG // SIb3DQEJEAIvMYHqMIHnMIHkMIG9BCDVKNe3BTGTeOjC
// SIG // OTXyAIPVMeXDucTPYp63ua4rjmfCLTCBmDCBgKR+MHwx
// SIG // CzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpXYXNoaW5ndG9u
// SIG // MRAwDgYDVQQHEwdSZWRtb25kMR4wHAYDVQQKExVNaWNy
// SIG // b3NvZnQgQ29ycG9yYXRpb24xJjAkBgNVBAMTHU1pY3Jv
// SIG // c29mdCBUaW1lLVN0YW1wIFBDQSAyMDEwAhMzAAACDLlk
// SIG // 4zWc7PSuAAEAAAIMMCIEIMiZHa88+Q4inCT9cr8QwTQn
// SIG // +cLECyjObxL9OjIwh8pqMA0GCSqGSIb3DQEBCwUABIIC
// SIG // AFT4DM0PbsUhPFa73+OwcLCf77Lc+D6c2EV85C34gxEv
// SIG // lNBoyelqXG9yx/1CPhXG3UQKWGubP3gE0O/8jT9+xwuq
// SIG // H/fjAV7Mcgim+CIPgNEhO+bjlRZDC/vyehM3jKNEIaSh
// SIG // GXFEEA2M+q3DUYEQBKeEFxLEAl8A5VWm0WxBmwfXxfZQ
// SIG // 9IJ+JpSNZW5HIoqZu03PruHnP5uzcQ7boRL128lHuyd3
// SIG // 4BPbq6SUc/bW3KHBZdtnS6arPB8LSJBQn/tjlgNy4Qem
// SIG // alnw2oEzaNNrTZokOlxXXxF1+3y7Yo4GyV1IDwefN8TB
// SIG // O1G/yhwhIIF9JwqnRwUoK2XOev6xUcXVA/cEVIKvDglH
// SIG // 6vwPveY4ey/Gdpn16gBV8cIzv35g/r01LPO3kjKb2QQo
// SIG // +F3FBu2DVsto+WOQmTbLj4/OR4YpuALSQnxr/fNeGGoz
// SIG // xgGn4FZ2fOj42Q0u9XqlJ29Tl+t4JJbzEeekaCRJysk+
// SIG // wwbEinz/gh1NTXxSUsQ0Fhbs/piUZefCurrm2/OMKGwN
// SIG // cdnary8d4AfzuZ9fezTZeDuBtBFbImvNimlflVfXlEKg
// SIG // h0Fva3wldqwubtLVa+EuYimCj15S9srD5HOVdFc46NX5
// SIG // Hu1Rbv7SoCbr5U/z+VH+t/kRnRcEFCSydPLcmfaeZHac
// SIG // LOtkM1TEryR1DvdoCyTAe9mX
// SIG // End signature block
