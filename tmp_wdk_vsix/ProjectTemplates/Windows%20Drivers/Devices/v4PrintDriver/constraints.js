/// <reference path="v4PrintDriver-Intellisense.js" />

// --------------------------------------------------------------------------
// Note: To disable intellisense for Windows 8.1 APIs, please delete the line below
/// <reference path="v4PrintDriver-Intellisense-Windows8.1.js" />
// --------------------------------------------------------------------------

var pskNs = "http://schemas.microsoft.com/windows/2003/08/printing/printschemakeywords";
var pskV11Ns = "http://schemas.microsoft.com/windows/2013/05/printing/printschemakeywordsv11";
var psfNs = "http://schemas.microsoft.com/windows/2003/08/printing/printschemaframework";

function validatePrintTicket(printTicket, scriptContext) {
    /// <param name="printTicket" type="IPrintSchemaTicket">
    ///     Print ticket to be validated.
    /// </param>
    /// <param name="scriptContext" type="IPrinterScriptContext">
    ///     Script context object.
    /// </param>
    /// <returns type="Number" integer="true">
    ///     Integer value indicating validation status.
    ///         1 - Print ticket is valid and was not modified.
    ///         2 - Print ticket was modified to make it valid.
    ///         0 - Print ticket is invalid.
    /// </returns>

    // Below demonstrates correct usage of IPrintSchemaTicket2 APIs so that the script does not terminate
    // when running on a Windows 8 version of PrintConfig.dll.
    if (printSchemaApiHelpers.supportsIPrintSchemaTicket2(printTicket)) {
        var param = printTicket.GetParameterInitializer("JobCopiesAllDocuments");
    }
}

function completePrintCapabilities(printTicket, scriptContext, printCapabilities) {
    /// <param name="printTicket" type="IPrintSchemaTicket" mayBeNull="true">
    ///     If not 'null', the print ticket's settings are used to customize the print capabilities.
    /// </param>
    /// <param name="scriptContext" type="IPrinterScriptContext">
    ///     Script context object.
    /// </param>
    /// <param name="printCapabilities" type="IPrintSchemaCapabilities">
    ///     Print capabilities object to be customized.
    /// </param>

    // Below demonstrates correct usage of IPrintSchemaCapabilities2 APIs so that the script does not terminate
    // when running on a Windows 8 version of PrintConfig.dll.
    if (printSchemaApiHelpers.supportsIPrintSchemaCapabilities2(printCapabilities)) {
        var param = printCapabilities.GetParameterDefinition("JobCopiesAllDocuments");
    }

}

function convertPrintTicketToDevMode(printTicket, scriptContext, devModeProperties) {
    /// <param name="printTicket" type="IPrintSchemaTicket">
    ///     Print ticket to be converted to DevMode.
    /// </param>
    /// <param name="scriptContext" type="IPrinterScriptContext">
    ///     Script context object.
    /// </param>
    /// <param name="devModeProperties" type="IPrinterScriptablePropertyBag">
    ///     The DevMode property bag.
    /// </param>
}


function convertDevModeToPrintTicket(devModeProperties, scriptContext, printTicket) {
    /// <param name="devModeProperties" type="IPrinterScriptablePropertyBag">
    ///     The DevMode property bag.
    /// </param>
    /// <param name="scriptContext" type="IPrinterScriptContext">
    ///     Script context object.
    /// </param>
    /// <param name="printTicket" type="IPrintSchemaTicket">
    ///     Print ticket to be converted from the DevMode.
    /// </param>

}

var PrintSchemaConstrainedSetting = {
    PrintSchemaConstrainedSetting_None: 0,
    PrintSchemaConstrainedSetting_PrintTicket: 1,
    PrintSchemaConstrainedSetting_Admin: 2,
    PrintSchemaConstrainedSetting_Device: 3
};

var PrintSchemaParameterDataType = {
    PrintSchemaParameterDataType_Integer: 0,
    PrintSchemaParameterDataType_NumericString: 1,
    PrintSchemaParameterDataType_String: 2
};

var STREAM_SEEK = {
    STREAM_SEEK_SET: 0,
    STREAM_SEEK_CUR: 1,
    STREAM_SEEK_END: 2
};

var PrintSchemaSelectionType = {
    PrintSchemaSelectionType_PickOne: 0,
    PrintSchemaSelectionType_PickMany: 1
};


var printSchemaApiHelpers = {
    supportsIPrintSchemaCapabilities2: function (printCapabilities) {
        /// <summary>
        ///     Determines if the IPrintSchemaCapabilities2 APIs are supported on the 'printCapabilities' object.
        /// </summary>
        /// <param name="printCapabilities" type="IPrintSchemaCapabilities">
        ///     Print capabilities object.
        /// </param>
        /// <returns type="Boolean">
        ///     true - the interface APIs are supported.
        ///     false - the interface APIs are not supported.
        /// </returns>

        var supported = true;

        try {
            if (typeof printCapabilities.getParameterDefinition === "undefined") {
                supported = false;
            }
        }
        catch (exception) {
            supported = false;
        }

        return supported;
    },
    supportsIPrintSchemaTicket2: function (printTicket) {
        /// <summary>
        ///     Determines if the IPrintSchemaTicket2 APIs are supported on the 'printTicket' object.
        /// </summary>
        /// <param name="printTicket" type="IPrintSchemaTicket">
        ///     Print ticket object.
        /// </param>
        /// <returns type="Boolean">
        ///     true - the interface APIs are supported.
        ///     false - the interface APIs are not supported.
        /// </returns>

        var supported = true;

        try {
            if (typeof printTicket.getParameterInitializer === "undefined") {
                supported = false;
            }
        }
        catch (exception) {
            supported = false;
        }

        return supported;
    }
}
// SIG // Begin signature block
// SIG // MIIsCQYJKoZIhvcNAQcCoIIr+jCCK/YCAQExDzANBglg
// SIG // hkgBZQMEAgEFADB3BgorBgEEAYI3AgEEoGkwZzAyBgor
// SIG // BgEEAYI3AgEeMCQCAQEEEBDgyQbOONQRoqMAEEvTUJAC
// SIG // AQACAQACAQACAQACAQAwMTANBglghkgBZQMEAgEFAAQg
// SIG // WYleTAOoRAAgXaHotaUuBOFULfUf7zc9MOIUv91ZRv2g
// SIG // ghF9MIIIjTCCB3WgAwIBAgITNgAAAg55l6Dp/V+f0wAC
// SIG // AAACDjANBgkqhkiG9w0BAQsFADBBMRMwEQYKCZImiZPy
// SIG // LGQBGRYDR0JMMRMwEQYKCZImiZPyLGQBGRYDQU1FMRUw
// SIG // EwYDVQQDEwxBTUUgQ1MgQ0EgMDEwHhcNMjUxMDIzMjMx
// SIG // MjAzWhcNMjYwNDI2MjMyMjAzWjAkMSIwIAYDVQQDExlN
// SIG // aWNyb3NvZnQgQXp1cmUgQ29kZSBTaWduMIIBIjANBgkq
// SIG // hkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAn68PZm44UaQs
// SIG // 9FofnZllOJ5QW3og5JeTXzVhQPTVlAHw3EgBLVoiU2Sy
// SIG // AVbfhOZI6cUHwnjm3cZNGmjypoZADZWuZCeXoI5q7bZh
// SIG // r7CP0j3nkPPdI/0DuGCOIEFqV57fBLMLVusc5kXTDD/g
// SIG // gEsgGCdKFkDFV1zDYQRvVQXa2Xvi9fg8y2sKuNgFhjaC
// SIG // i+iXRoNtT9FtdTDdTrcK4bm4E6hAZIGKe5tQbeBHjsQc
// SIG // 9BsGXzzg2y1U25QAki5hpXxHjxHVWpqiFE03bKwChvDT
// SIG // P4BsdNdAIqgL+qdrArUnmab1e35KaS2PWo3ZvGHOCn9S
// SIG // jSN1OWTTHSyA8/XwOUPrpQIDAQABo4IFmTCCBZUwKQYJ
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
// SIG // bkF1dGhvcml0eTAdBgNVHQ4EFgQUmyia4LlYRWoI9bUm
// SIG // 8xZ7RxrboAkwDgYDVR0PAQH/BAQDAgeAMFQGA1UdEQRN
// SIG // MEukSTBHMS0wKwYDVQQLEyRNaWNyb3NvZnQgSXJlbGFu
// SIG // ZCBPcGVyYXRpb25zIExpbWl0ZWQxFjAUBgNVBAUTDTIz
// SIG // NjE2Nys1MDYwNTIwggHmBgNVHR8EggHdMIIB2TCCAdWg
// SIG // ggHRoIIBzYY/aHR0cDovL2NybC5taWNyb3NvZnQuY29t
// SIG // L3BraWluZnJhL0NSTC9BTUUlMjBDUyUyMENBJTIwMDEo
// SIG // MikuY3JshjFodHRwOi8vY3JsMS5hbWUuZ2JsL2NybC9B
// SIG // TUUlMjBDUyUyMENBJTIwMDEoMikuY3JshjFodHRwOi8v
// SIG // Y3JsMi5hbWUuZ2JsL2NybC9BTUUlMjBDUyUyMENBJTIw
// SIG // MDEoMikuY3JshjFodHRwOi8vY3JsMy5hbWUuZ2JsL2Ny
// SIG // bC9BTUUlMjBDUyUyMENBJTIwMDEoMikuY3JshjFodHRw
// SIG // Oi8vY3JsNC5hbWUuZ2JsL2NybC9BTUUlMjBDUyUyMENB
// SIG // JTIwMDEoMikuY3JshoG9bGRhcDovLy9DTj1BTUUlMjBD
// SIG // UyUyMENBJTIwMDEoMiksQ049QlkyUEtJQ1NDQTAxLENO
// SIG // PUNEUCxDTj1QdWJsaWMlMjBLZXklMjBTZXJ2aWNlcyxD
// SIG // Tj1TZXJ2aWNlcyxDTj1Db25maWd1cmF0aW9uLERDPUFN
// SIG // RSxEQz1HQkw/Y2VydGlmaWNhdGVSZXZvY2F0aW9uTGlz
// SIG // dD9iYXNlP29iamVjdENsYXNzPWNSTERpc3RyaWJ1dGlv
// SIG // blBvaW50MB8GA1UdIwQYMBaAFJZRhOBrb3v+2Aarw/KF
// SIG // 5imuavnUMB8GA1UdJQQYMBYGCisGAQQBgjdbAQEGCCsG
// SIG // AQUFBwMDMA0GCSqGSIb3DQEBCwUAA4IBAQCmgYfwfONl
// SIG // DxajRz/ptpmEmPNKKyoz1Z1xG/ipQqMS9SJJdg384hNT
// SIG // wWV7Qg22Lb8jKgvT4yrwfEzc+9fglqztJaldn9CiEzJr
// SIG // V4gsvsk4Qai2L1Ga260c5V2L9pUJqljMtDqqykyHTM2t
// SIG // osFKdoHS5ysxuw0PCHG4LmF8MIfhs23lI2PYRIu7F+3Y
// SIG // FT+C7NVEDZEakRXETzgKEwWbwRoT0pDgJ5fQkobwh3yq
// SIG // x5dMswZP5P5Z7eA4ugOsFquBMNQb3h1zB9WU/rHUbopJ
// SIG // uWCy6i3Px4aIk964neRB1UCt7J4PeXyHQTII9ukknDL8
// SIG // iljQvYCpKXpns1EjP3PdcBIcMIII6DCCBtCgAwIBAgIT
// SIG // HwAAAFHqj/accwyoOwAAAAAAUTANBgkqhkiG9w0BAQsF
// SIG // ADA8MRMwEQYKCZImiZPyLGQBGRYDR0JMMRMwEQYKCZIm
// SIG // iZPyLGQBGRYDQU1FMRAwDgYDVQQDEwdhbWVyb290MB4X
// SIG // DTIxMDUyMTE4NDQxNFoXDTI2MDUyMTE4NTQxNFowQTET
// SIG // MBEGCgmSJomT8ixkARkWA0dCTDETMBEGCgmSJomT8ixk
// SIG // ARkWA0FNRTEVMBMGA1UEAxMMQU1FIENTIENBIDAxMIIB
// SIG // IjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAyZpS
// SIG // CX0Bno1W1yqXMhT6BUlJZWpa4p3xFeiTHO4vm2Q6C/az
// SIG // R5xwxnyYHrkSGDtS2P9X+KDE64V20mmEQkubxnPNeOVn
// SIG // E2RvdPGxgwlq+BhS3ONdVsQPj79q7XgHM9HhzB9+qk0P
// SIG // C9KN1zm9p/seyiRS6JF1dbOqRf1pUl7FAVxmgiCFgV8h
// SIG // HIb/rDPXig7FDi3S0yEx2CUDVpIq8jEhG8anUFE1WYxM
// SIG // +ni0S5KHwwKPKV4qyGDoDO+9AmDoma3Chyu5WDlW5cdt
// SIG // qXTWsGPE3umtnX6AmlldUFLms4OVR4guKf+n5LIBCC6b
// SIG // TiocfXPomqYjYTKx7AGMfaVLaaXmhQIDAQABo4IE3DCC
// SIG // BNgwEgYJKwYBBAGCNxUBBAUCAwIAAjAjBgkrBgEEAYI3
// SIG // FQIEFgQUEmgkQiFHy9RrvjHPIKTACyN/P0cwHQYDVR0O
// SIG // BBYEFJZRhOBrb3v+2Aarw/KF5imuavnUMIIBBAYDVR0l
// SIG // BIH8MIH5BgcrBgEFAgMFBggrBgEFBQcDAQYIKwYBBQUH
// SIG // AwIGCisGAQQBgjcUAgEGCSsGAQQBgjcVBgYKKwYBBAGC
// SIG // NwoDDAYJKwYBBAGCNxUGBggrBgEFBQcDCQYIKwYBBQUI
// SIG // AgIGCisGAQQBgjdAAQEGCysGAQQBgjcKAwQBBgorBgEE
// SIG // AYI3CgMEBgkrBgEEAYI3FQUGCisGAQQBgjcUAgIGCisG
// SIG // AQQBgjcUAgMGCCsGAQUFBwMDBgorBgEEAYI3WwEBBgor
// SIG // BgEEAYI3WwIBBgorBgEEAYI3WwMBBgorBgEEAYI3WwUB
// SIG // BgorBgEEAYI3WwQBBgorBgEEAYI3WwQCMBkGCSsGAQQB
// SIG // gjcUAgQMHgoAUwB1AGIAQwBBMAsGA1UdDwQEAwIBhjAS
// SIG // BgNVHRMBAf8ECDAGAQH/AgEAMB8GA1UdIwQYMBaAFCle
// SIG // UV5krjS566ycDaeMdQHRCQsoMIIBaAYDVR0fBIIBXzCC
// SIG // AVswggFXoIIBU6CCAU+GMWh0dHA6Ly9jcmwubWljcm9z
// SIG // b2Z0LmNvbS9wa2lpbmZyYS9jcmwvYW1lcm9vdC5jcmyG
// SIG // I2h0dHA6Ly9jcmwyLmFtZS5nYmwvY3JsL2FtZXJvb3Qu
// SIG // Y3JshiNodHRwOi8vY3JsMy5hbWUuZ2JsL2NybC9hbWVy
// SIG // b290LmNybIYjaHR0cDovL2NybDEuYW1lLmdibC9jcmwv
// SIG // YW1lcm9vdC5jcmyGgapsZGFwOi8vL0NOPWFtZXJvb3Qs
// SIG // Q049QU1FUm9vdCxDTj1DRFAsQ049UHVibGljJTIwS2V5
// SIG // JTIwU2VydmljZXMsQ049U2VydmljZXMsQ049Q29uZmln
// SIG // dXJhdGlvbixEQz1BTUUsREM9R0JMP2NlcnRpZmljYXRl
// SIG // UmV2b2NhdGlvbkxpc3Q/YmFzZT9vYmplY3RDbGFzcz1j
// SIG // UkxEaXN0cmlidXRpb25Qb2ludDCCAasGCCsGAQUFBwEB
// SIG // BIIBnTCCAZkwRwYIKwYBBQUHMAKGO2h0dHA6Ly9jcmwu
// SIG // bWljcm9zb2Z0LmNvbS9wa2lpbmZyYS9jZXJ0cy9BTUVS
// SIG // b290X2FtZXJvb3QuY3J0MDcGCCsGAQUFBzAChitodHRw
// SIG // Oi8vY3JsMi5hbWUuZ2JsL2FpYS9BTUVSb290X2FtZXJv
// SIG // b3QuY3J0MDcGCCsGAQUFBzAChitodHRwOi8vY3JsMy5h
// SIG // bWUuZ2JsL2FpYS9BTUVSb290X2FtZXJvb3QuY3J0MDcG
// SIG // CCsGAQUFBzAChitodHRwOi8vY3JsMS5hbWUuZ2JsL2Fp
// SIG // YS9BTUVSb290X2FtZXJvb3QuY3J0MIGiBggrBgEFBQcw
// SIG // AoaBlWxkYXA6Ly8vQ049YW1lcm9vdCxDTj1BSUEsQ049
// SIG // UHVibGljJTIwS2V5JTIwU2VydmljZXMsQ049U2Vydmlj
// SIG // ZXMsQ049Q29uZmlndXJhdGlvbixEQz1BTUUsREM9R0JM
// SIG // P2NBQ2VydGlmaWNhdGU/YmFzZT9vYmplY3RDbGFzcz1j
// SIG // ZXJ0aWZpY2F0aW9uQXV0aG9yaXR5MA0GCSqGSIb3DQEB
// SIG // CwUAA4ICAQBQECO3Tw/o317Rrd7yadqcswPx1LvIYymk
// SIG // aTN6KcmuRt6HKa0Xe73Ux2/AQ30TfgA9GBJngweRykKB
// SIG // usRzyOU17iIubJvy3gA21dwtqtB0DsoEv1U/ptVu2v++
// SIG // doTCJ/i+GbssVXkgaX8H+6EOGEmT4evp4GbwR4HwWlc+
// SIG // Dvf8HH8PdUA2Z04CvcwIfckSipbNm84jxJ8XjmTFTWsc
// SIG // ldL9edj2NsY6iGnyJFIyur2PS7VRYyV3p1VAJp91gj1j
// SIG // RQtWEyCB8P5g9nE3z8u0ANaU/hjwEQCrdGyravWgnf2J
// SIG // tG+bT26YAokbc8m+32zUtXRO+NK3tAjhOu2FdsG3qNrF
// SIG // 4sc7y37R/C+7Pcb/cFfhttqsirepZii4xStcjMODYuXz
// SIG // Gm3IJs0b0owHG6oKd7ZOGvHpmmh9K8/DLriD/sq8bURD
// SIG // 10qi/wuW8zM7IpLg1vcR9dIK2mc0pj44pc6UX0XbttP/
// SIG // VEJgu3lT2eI9VjWtaKjx38xE9woSMyekPRtzTwgfuysF
// SIG // 9DkJisr+yA4po/FPxpbBw9c/hBf32DH/GFxteS2pmjgK
// SIG // IbMP8sDukmEq3lVvuWNJsybrZwQvQpvaM49fv+JKpLK5
// SIG // YWYEfwksYRR9wU8Hh/ID9hRCEkbUoQ2W7mMpsp2Nbp/k
// SIG // cn4ivfolUy3Q9Yf0scsQ6WTLYpm+AoCUJTGCGeQwghng
// SIG // AgEBMFgwQTETMBEGCgmSJomT8ixkARkWA0dCTDETMBEG
// SIG // CgmSJomT8ixkARkWA0FNRTEVMBMGA1UEAxMMQU1FIENT
// SIG // IENBIDAxAhM2AAACDnmXoOn9X5/TAAIAAAIOMA0GCWCG
// SIG // SAFlAwQCAQUAoIGuMBkGCSqGSIb3DQEJAzEMBgorBgEE
// SIG // AYI3AgEEMBwGCisGAQQBgjcCAQsxDjAMBgorBgEEAYI3
// SIG // AgEVMC8GCSqGSIb3DQEJBDEiBCCkf1q5sglDyh79yeX3
// SIG // fd1yFSUlo0D6lipk5DdNUnIZUTBCBgorBgEEAYI3AgEM
// SIG // MTQwMqAUgBIATQBpAGMAcgBvAHMAbwBmAHShGoAYaHR0
// SIG // cDovL3d3dy5taWNyb3NvZnQuY29tMA0GCSqGSIb3DQEB
// SIG // AQUABIIBAIlB8jKAL16L81CmZndLPkE8JEcNHH5G8s79
// SIG // D8dVGSmg/jJp81/FQI2yp8Q/siyxZd4KVvk3/iOSJxTr
// SIG // vBQmWArn3d+NS4MsGvaIeE4ukEkAHanEYEK0f8AeYjkp
// SIG // SReb7nw4RlA/UE4UZZx4t10BOLpjzRw+Uy3XKHoHY0dE
// SIG // PQ7U69Zd3X3EoOLlPJF6j7Hz/J3BWQZ7Wv6DE3a7B6JL
// SIG // op8DlnBtMsiRBmb859fAc4HkdCUBn2L9b5FAriU8U2y7
// SIG // 27aX6qnE7O2qsC86QJQc5ceWB68/9L1gXN0uH9oDs1Ko
// SIG // ITuM4QOeU0/BDaZqOwHnLPjKDU8FuMDc/r2+towlJFOh
// SIG // ghesMIIXqAYKKwYBBAGCNwMDATGCF5gwgheUBgkqhkiG
// SIG // 9w0BBwKggheFMIIXgQIBAzEPMA0GCWCGSAFlAwQCAQUA
// SIG // MIIBWQYLKoZIhvcNAQkQAQSgggFIBIIBRDCCAUACAQEG
// SIG // CisGAQQBhFkKAwEwMTANBglghkgBZQMEAgEFAAQgNvZm
// SIG // 1e35IcbnXZEhW1s8L/ZkG9Kwcp4RFioJvePKO8oCBmkk
// SIG // 6AcNphgSMjAyNTEyMDIxNzM2MjQuNzlaMASAAgH0oIHZ
// SIG // pIHWMIHTMQswCQYDVQQGEwJVUzETMBEGA1UECBMKV2Fz
// SIG // aGluZ3RvbjEQMA4GA1UEBxMHUmVkbW9uZDEeMBwGA1UE
// SIG // ChMVTWljcm9zb2Z0IENvcnBvcmF0aW9uMS0wKwYDVQQL
// SIG // EyRNaWNyb3NvZnQgSXJlbGFuZCBPcGVyYXRpb25zIExp
// SIG // bWl0ZWQxJzAlBgNVBAsTHm5TaGllbGQgVFNTIEVTTjoy
// SIG // QTFBLTA1RTAtRDk0NzElMCMGA1UEAxMcTWljcm9zb2Z0
// SIG // IFRpbWUtU3RhbXAgU2VydmljZaCCEfswggcoMIIFEKAD
// SIG // AgECAhMzAAACEKvN5BYY7zmwAAEAAAIQMA0GCSqGSIb3
// SIG // DQEBCwUAMHwxCzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpX
// SIG // YXNoaW5ndG9uMRAwDgYDVQQHEwdSZWRtb25kMR4wHAYD
// SIG // VQQKExVNaWNyb3NvZnQgQ29ycG9yYXRpb24xJjAkBgNV
// SIG // BAMTHU1pY3Jvc29mdCBUaW1lLVN0YW1wIFBDQSAyMDEw
// SIG // MB4XDTI1MDgxNDE4NDgxMloXDTI2MTExMzE4NDgxMlow
// SIG // gdMxCzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpXYXNoaW5n
// SIG // dG9uMRAwDgYDVQQHEwdSZWRtb25kMR4wHAYDVQQKExVN
// SIG // aWNyb3NvZnQgQ29ycG9yYXRpb24xLTArBgNVBAsTJE1p
// SIG // Y3Jvc29mdCBJcmVsYW5kIE9wZXJhdGlvbnMgTGltaXRl
// SIG // ZDEnMCUGA1UECxMeblNoaWVsZCBUU1MgRVNOOjJBMUEt
// SIG // MDVFMC1EOTQ3MSUwIwYDVQQDExxNaWNyb3NvZnQgVGlt
// SIG // ZS1TdGFtcCBTZXJ2aWNlMIICIjANBgkqhkiG9w0BAQEF
// SIG // AAOCAg8AMIICCgKCAgEAjcc4q057ZwIgpKu4pTXWLejv
// SIG // YEduRf+1mIpbiJEMFWWmU2xpip+zK7xFxKGB1CclUXBU
// SIG // 0/ZQZ6LG8H0gI7yvosrsPEI1DPB/XccGCvswKbAKckng
// SIG // OuGTEPGk7K/vEZa9h0Xt02b7m2n9MdIjkLrFl0pDriKy
// SIG // z0QHGpdh93X6+NApfE1TL24Vo0xkeoFGpL3rX9gXhIOF
// SIG // 59EMnTd2o45FW/oxMgY9q0y0jGO0HrCLTCZr50e7TZRS
// SIG // NYAy2lyKbvKI2MKlN1wLzJvZbbc//L3s1q3J6KhS0KC2
// SIG // VNEImYdFgVkJej4zZqHfScTbx9hjFgFpVkJl4xH5VJ8t
// SIG // yJdXE9+vU0k9AaT2QP1Zm3WQmXedSoLjjI7LWznuHwno
// SIG // GIXLiJMQzPqKqRIFL3wzcrDrZeWgtAdBPbipglZ5CQns
// SIG // 6Baj5Mb6a/EZC9G3faJYK5QVHeE6eLoSEwp1dz5WurLX
// SIG // NPsp0VWplpl/FJb8jrRT/jOoHu85qRcdYpgByU9W7IWP
// SIG // drthmyfqeAw0omVWN5JxcogYbLo2pANJHlsMdWnxIpN5
// SIG // YwHbGEPCuosBHPk2Xd9+E/pZPQUR6v+D85eEN5A/ZM/x
// SIG // iPpxa8dJZ87BpTvui7/2uflUMJf2Yc9ZLPgEdhQQo0Lw
// SIG // MDSTDT48y3sV7Pdo+g5q+MqnJztN/6qt1cgUTe9u+ykC
// SIG // AwEAAaOCAUkwggFFMB0GA1UdDgQWBBSe42+FrpdF2avb
// SIG // Uhlk86BLSH5kejAfBgNVHSMEGDAWgBSfpxVdAF5iXYP0
// SIG // 5dJlpxtTNRnpcjBfBgNVHR8EWDBWMFSgUqBQhk5odHRw
// SIG // Oi8vd3d3Lm1pY3Jvc29mdC5jb20vcGtpb3BzL2NybC9N
// SIG // aWNyb3NvZnQlMjBUaW1lLVN0YW1wJTIwUENBJTIwMjAx
// SIG // MCgxKS5jcmwwbAYIKwYBBQUHAQEEYDBeMFwGCCsGAQUF
// SIG // BzAChlBodHRwOi8vd3d3Lm1pY3Jvc29mdC5jb20vcGtp
// SIG // b3BzL2NlcnRzL01pY3Jvc29mdCUyMFRpbWUtU3RhbXAl
// SIG // MjBQQ0ElMjAyMDEwKDEpLmNydDAMBgNVHRMBAf8EAjAA
// SIG // MBYGA1UdJQEB/wQMMAoGCCsGAQUFBwMIMA4GA1UdDwEB
// SIG // /wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAgEAvs4rO3oo
// SIG // 8czOrxPqnnSEkUVq718QzlrIiy7/EW7JmQXsJoFxHWUF
// SIG // 0Ux0PDyKFDRXPJVv29F7kpJkBJJmcQg5HQV7blUXIMWQ
// SIG // 1qX0KdtFQXI/MRL77Z+pK5x1jX+tbRkA7a5Ft7vWuRoA
// SIG // Ei02HpFH5m/Akh/dfsbx8wOpecJbYvuHuy4aG0/tGzOW
// SIG // FCxMMNhGAIJ4qdV87JnY/uMBmiodlm+Gz357XWW5tg3H
// SIG // rtNZXuQ0tWUv26ud4nGKJo/oLZHP75p4Rpt7dMdYKUF9
// SIG // AuVFBwxYZYpvgk12tfK+/yOwq84/fjXVCdM83Qnawtbe
// SIG // nbk/lnbc9KsZom+GnvA4itAMUpSXFWrcRkqdUQLN+JrG
// SIG // 6fPBoV8+D8U2Q2F4XkiCR6EU9JzYKwTuvL6t3nFuxnkL
// SIG // dNjbTg2/yv2j3WaDuCK5lSPgsndIiH6Bku2Ui3A0aUo6
// SIG // D9z9v+XEuBs9ioVJaOjf/z+Urqg7ESnxG0/T1dKci7vL
// SIG // Q2XNgWFYO+/OlDjtGoma1ijX4m14N9qgrXTuWEGwgC7h
// SIG // hBgp3id/LAOf9BSTWA5lBrilsEoexXBrOn/1wM3rjG0h
// SIG // IsxvF5/YOK78mVRGY6Y7zYJ+uXt4OTOFBwadPv8Mklre
// SIG // QZLPnQPtiwop4rlLUYaPCiD4YUqRNbLp8Sgyo9g0iAcZ
// SIG // YznTuc+8Q8ZIrgwwggdxMIIFWaADAgECAhMzAAAAFcXn
// SIG // a54Cm0mZAAAAAAAVMA0GCSqGSIb3DQEBCwUAMIGIMQsw
// SIG // CQYDVQQGEwJVUzETMBEGA1UECBMKV2FzaGluZ3RvbjEQ
// SIG // MA4GA1UEBxMHUmVkbW9uZDEeMBwGA1UEChMVTWljcm9z
// SIG // b2Z0IENvcnBvcmF0aW9uMTIwMAYDVQQDEylNaWNyb3Nv
// SIG // ZnQgUm9vdCBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkgMjAx
// SIG // MDAeFw0yMTA5MzAxODIyMjVaFw0zMDA5MzAxODMyMjVa
// SIG // MHwxCzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpXYXNoaW5n
// SIG // dG9uMRAwDgYDVQQHEwdSZWRtb25kMR4wHAYDVQQKExVN
// SIG // aWNyb3NvZnQgQ29ycG9yYXRpb24xJjAkBgNVBAMTHU1p
// SIG // Y3Jvc29mdCBUaW1lLVN0YW1wIFBDQSAyMDEwMIICIjAN
// SIG // BgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEA5OGmTOe0
// SIG // ciELeaLL1yR5vQ7VgtP97pwHB9KpbE51yMo1V/YBf2xK
// SIG // 4OK9uT4XYDP/XE/HZveVU3Fa4n5KWv64NmeFRiMMtY0T
// SIG // z3cywBAY6GB9alKDRLemjkZrBxTzxXb1hlDcwUTIcVxR
// SIG // MTegCjhuje3XD9gmU3w5YQJ6xKr9cmmvHaus9ja+NSZk
// SIG // 2pg7uhp7M62AW36MEBydUv626GIl3GoPz130/o5Tz9bs
// SIG // hVZN7928jaTjkY+yOSxRnOlwaQ3KNi1wjjHINSi947SH
// SIG // JMPgyY9+tVSP3PoFVZhtaDuaRr3tpK56KTesy+uDRedG
// SIG // bsoy1cCGMFxPLOJiss254o2I5JasAUq7vnGpF1tnYN74
// SIG // kpEeHT39IM9zfUGaRnXNxF803RKJ1v2lIH1+/NmeRd+2
// SIG // ci/bfV+AutuqfjbsNkz2K26oElHovwUDo9Fzpk03dJQc
// SIG // NIIP8BDyt0cY7afomXw/TNuvXsLz1dhzPUNOwTM5TI4C
// SIG // vEJoLhDqhFFG4tG9ahhaYQFzymeiXtcodgLiMxhy16cg
// SIG // 8ML6EgrXY28MyTZki1ugpoMhXV8wdJGUlNi5UPkLiWHz
// SIG // NgY1GIRH29wb0f2y1BzFa/ZcUlFdEtsluq9QBXpsxREd
// SIG // cu+N+VLEhReTwDwV2xo3xwgVGD94q0W29R6HXtqPnhZy
// SIG // acaue7e3PmriLq0CAwEAAaOCAd0wggHZMBIGCSsGAQQB
// SIG // gjcVAQQFAgMBAAEwIwYJKwYBBAGCNxUCBBYEFCqnUv5k
// SIG // xJq+gpE8RjUpzxD/LwTuMB0GA1UdDgQWBBSfpxVdAF5i
// SIG // XYP05dJlpxtTNRnpcjBcBgNVHSAEVTBTMFEGDCsGAQQB
// SIG // gjdMg30BATBBMD8GCCsGAQUFBwIBFjNodHRwOi8vd3d3
// SIG // Lm1pY3Jvc29mdC5jb20vcGtpb3BzL0RvY3MvUmVwb3Np
// SIG // dG9yeS5odG0wEwYDVR0lBAwwCgYIKwYBBQUHAwgwGQYJ
// SIG // KwYBBAGCNxQCBAweCgBTAHUAYgBDAEEwCwYDVR0PBAQD
// SIG // AgGGMA8GA1UdEwEB/wQFMAMBAf8wHwYDVR0jBBgwFoAU
// SIG // 1fZWy4/oolxiaNE9lJBb186aGMQwVgYDVR0fBE8wTTBL
// SIG // oEmgR4ZFaHR0cDovL2NybC5taWNyb3NvZnQuY29tL3Br
// SIG // aS9jcmwvcHJvZHVjdHMvTWljUm9vQ2VyQXV0XzIwMTAt
// SIG // MDYtMjMuY3JsMFoGCCsGAQUFBwEBBE4wTDBKBggrBgEF
// SIG // BQcwAoY+aHR0cDovL3d3dy5taWNyb3NvZnQuY29tL3Br
// SIG // aS9jZXJ0cy9NaWNSb29DZXJBdXRfMjAxMC0wNi0yMy5j
// SIG // cnQwDQYJKoZIhvcNAQELBQADggIBAJ1VffwqreEsH2cB
// SIG // MSRb4Z5yS/ypb+pcFLY+TkdkeLEGk5c9MTO1OdfCcTY/
// SIG // 2mRsfNB1OW27DzHkwo/7bNGhlBgi7ulmZzpTTd2YurYe
// SIG // eNg2LpypglYAA7AFvonoaeC6Ce5732pvvinLbtg/SHUB
// SIG // 2RjebYIM9W0jVOR4U3UkV7ndn/OOPcbzaN9l9qRWqveV
// SIG // tihVJ9AkvUCgvxm2EhIRXT0n4ECWOKz3+SmJw7wXsFSF
// SIG // QrP8DJ6LGYnn8AtqgcKBGUIZUnWKNsIdw2FzLixre24/
// SIG // LAl4FOmRsqlb30mjdAy87JGA0j3mSj5mO0+7hvoyGtmW
// SIG // 9I/2kQH2zsZ0/fZMcm8Qq3UwxTSwethQ/gpY3UA8x1Rt
// SIG // nWN0SCyxTkctwRQEcb9k+SS+c23Kjgm9swFXSVRk2XPX
// SIG // fx5bRAGOWhmRaw2fpCjcZxkoJLo4S5pu+yFUa2pFEUep
// SIG // 8beuyOiJXk+d0tBMdrVXVAmxaQFEfnyhYWxz/gq77EFm
// SIG // PWn9y8FBSX5+k77L+DvktxW/tM4+pTFRhLy/AsGConsX
// SIG // HRWJjXD+57XQKBqJC4822rpM+Zv/Cuk0+CQ1ZyvgDbjm
// SIG // jJnW4SLq8CdCPSWU5nR0W2rRnj7tfqAxM328y+l7vzhw
// SIG // RNGQ8cirOoo6CGJ/2XBjU02N7oJtpQUQwXEGahC0HVUz
// SIG // WLOhcGbyoYIDVjCCAj4CAQEwggEBoYHZpIHWMIHTMQsw
// SIG // CQYDVQQGEwJVUzETMBEGA1UECBMKV2FzaGluZ3RvbjEQ
// SIG // MA4GA1UEBxMHUmVkbW9uZDEeMBwGA1UEChMVTWljcm9z
// SIG // b2Z0IENvcnBvcmF0aW9uMS0wKwYDVQQLEyRNaWNyb3Nv
// SIG // ZnQgSXJlbGFuZCBPcGVyYXRpb25zIExpbWl0ZWQxJzAl
// SIG // BgNVBAsTHm5TaGllbGQgVFNTIEVTTjoyQTFBLTA1RTAt
// SIG // RDk0NzElMCMGA1UEAxMcTWljcm9zb2Z0IFRpbWUtU3Rh
// SIG // bXAgU2VydmljZaIjCgEBMAcGBSsOAwIaAxUAOsyf2b6r
// SIG // iPKnnXlIgIL2f53PUsKggYMwgYCkfjB8MQswCQYDVQQG
// SIG // EwJVUzETMBEGA1UECBMKV2FzaGluZ3RvbjEQMA4GA1UE
// SIG // BxMHUmVkbW9uZDEeMBwGA1UEChMVTWljcm9zb2Z0IENv
// SIG // cnBvcmF0aW9uMSYwJAYDVQQDEx1NaWNyb3NvZnQgVGlt
// SIG // ZS1TdGFtcCBQQ0EgMjAxMDANBgkqhkiG9w0BAQsFAAIF
// SIG // AOzZSV0wIhgPMjAyNTEyMDIxMTE3NDlaGA8yMDI1MTIw
// SIG // MzExMTc0OVowdDA6BgorBgEEAYRZCgQBMSwwKjAKAgUA
// SIG // 7NlJXQIBADAHAgEAAgIW2TAHAgEAAgIS+DAKAgUA7Nqa
// SIG // 3QIBADA2BgorBgEEAYRZCgQCMSgwJjAMBgorBgEEAYRZ
// SIG // CgMCoAowCAIBAAIDB6EgoQowCAIBAAIDAYagMA0GCSqG
// SIG // SIb3DQEBCwUAA4IBAQDF8hddn3VuqeqL+Yn0fe3oFtIK
// SIG // tGPUFlW2MjYyhO7A/k1w9gtm0sTIwM9ern/BTZnkkjc6
// SIG // wZ4SF4bv+PxtAAMO1wBoLdq7kqpT7SfGtw7kV/whiDuI
// SIG // TPatyrLWCZVqwkXfBT+wP24oWmM7SRcqMorfmzgiu+Jw
// SIG // SHo/KcQeGhTsCDKwc7Rh0ktpxikq/CwkpNv5L8wpT+GX
// SIG // yEE+8ll/Wr+z432CfZ03y8kmoITerNXoIPZejGqsu7bv
// SIG // bNnn8l/57qkxdRKNtr27tlEjGdvkHyztEaGhDvLJn4a3
// SIG // 2PJqtv30oMPYqdjo/DKZ4jrOYYBSurKzaz5VNjtEA4wa
// SIG // qX6pH92nMYIEDTCCBAkCAQEwgZMwfDELMAkGA1UEBhMC
// SIG // VVMxEzARBgNVBAgTCldhc2hpbmd0b24xEDAOBgNVBAcT
// SIG // B1JlZG1vbmQxHjAcBgNVBAoTFU1pY3Jvc29mdCBDb3Jw
// SIG // b3JhdGlvbjEmMCQGA1UEAxMdTWljcm9zb2Z0IFRpbWUt
// SIG // U3RhbXAgUENBIDIwMTACEzMAAAIQq83kFhjvObAAAQAA
// SIG // AhAwDQYJYIZIAWUDBAIBBQCgggFKMBoGCSqGSIb3DQEJ
// SIG // AzENBgsqhkiG9w0BCRABBDAvBgkqhkiG9w0BCQQxIgQg
// SIG // X4MBFga7/p8Po/XQdhWD3oHpIDS+lZDr9SG1R/rb08sw
// SIG // gfoGCyqGSIb3DQEJEAIvMYHqMIHnMIHkMIG9BCDD1SHu
// SIG // fsjzY59S1iHUQY9hnsKSrJPg5a9Mc4YnGmPHxjCBmDCB
// SIG // gKR+MHwxCzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpXYXNo
// SIG // aW5ndG9uMRAwDgYDVQQHEwdSZWRtb25kMR4wHAYDVQQK
// SIG // ExVNaWNyb3NvZnQgQ29ycG9yYXRpb24xJjAkBgNVBAMT
// SIG // HU1pY3Jvc29mdCBUaW1lLVN0YW1wIFBDQSAyMDEwAhMz
// SIG // AAACEKvN5BYY7zmwAAEAAAIQMCIEIAwrUsB7+WyHFxlb
// SIG // Jy2K7S0h9R5MSjf+eX0q3/2i6ihLMA0GCSqGSIb3DQEB
// SIG // CwUABIICADFvGM5ut9XELh8O3R58o2EWBB4W/7ZV4mfr
// SIG // ZHtAklSjsl0dJEWc6XxUkBI/2DOrAu4mnyINzT5wvE9s
// SIG // qp34CXiZgbRTAMMgedSwsHvLWkjpTzzpUCEtEiyKJdPk
// SIG // W4ao7eOJ6Y4afDuUoZpnP0jY+6//wqpBXr6lTCkenrCM
// SIG // VPZZ2ughdtdvluOMNLhBYCJQ6uaTi+3wvYYJopPqWtZY
// SIG // IbTnNVrBUmiok4hKJVwQWIlpnITdgZlRlUlFyJ4Ag99x
// SIG // 0/3Mwdl2aqxWOB2B82sreWHfKr5Jk/Y2jYpf+9JgjfGe
// SIG // AJQndp+wzkXmRu/SwCt4KxCC9sqocbtOFKrdJs7QIEta
// SIG // NugCjCcX1XP+x0zSmA/6G+8BKIJgn2V9FX9Sho83lB3Q
// SIG // e1T2Blkx0Ee3+rkUSGegT8+az+wFDp7bUZq24VP/MkxB
// SIG // oAx2Z//lBskQzFOYu+U50qH4t5tsuaE5fn4hm0EIZnQL
// SIG // 0iUSuC8gTfKXcFKFoqvHTIji+DB6r82L9igBeK6nzvqb
// SIG // QxsYXFI5t5RHVKnRAq69Fje3CHThNTq8f76INdifB8yW
// SIG // nUFlFC2hk1yGkgGnc26ibmruFLrDKTrjbMFVzuvtsSVO
// SIG // xBv85ruJqIkrisr26lCi2T4en1CtAZgEgI4yN3BU1rx9
// SIG // Jzt0V3diaa9OQwvczDDs4VNLNn1zUDNE
// SIG // End signature block
