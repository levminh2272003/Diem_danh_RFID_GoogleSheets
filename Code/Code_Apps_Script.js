function doGet(e) { 
  Logger.log(JSON.stringify(e));
  var result = 'Ok';
  if (!e.parameter || Object.keys(e.parameter).length === 0) {
    result = 'No Parameters';
  } else {
    var sheet_id = '1TMdENVgO0h0lhvGF0VcJ5XL-2Ht1zwBXckba3NGJz7c';  // Spreadsheet ID.
    var sheet_name = "DANH_SACH";  // Sheet Name in Google Sheets.

    var sheet_open = SpreadsheetApp.openById(sheet_id);
    var sheet_target = sheet_open.getSheetByName(sheet_name);

    var rowDataLog = [];

    var Curr_Date = Utilities.formatDate(new Date(), "Asia/Jakarta", 'dd/MM/yyyy');
    rowDataLog[0] = Curr_Date;  // Date -> column A

    var Curr_Time = Utilities.formatDate(new Date(), "Asia/Jakarta", 'HH:mm:ss');
    rowDataLog[1] = Curr_Time;  // Time -> column B

    var sts_val = '';

    for (var param in e.parameter) {
      Logger.log('In for loop, param=' + param);
      var value = stripQuotes(e.parameter[param]);
      Logger.log(param + ':' + e.parameter[param]);
      switch (param) {
        case 'sts':
          sts_val = value;
          break;
        case 'uid':
          rowDataLog[2] = value;  // MA_RFID
          result += ', UID Written';
          break;
        case 'name':
          rowDataLog[3] = value;  // HO_VA_TEN
          result += ', Name Written';
          break; 
        case 'masv':
          rowDataLog[4] = value;  // MA_SV
          result += ', MASV Written';
          break;
        case 'inout':
          rowDataLog[5] = value;  // TRANG_THAI
          result += ', INOUT Written';
          break;       
        default:
          result += ", unsupported parameter";
      }
    }

    if (sts_val == 'writeuid') {
      Logger.log(JSON.stringify(rowDataLog));
      if (Array.isArray(rowDataLog) && rowDataLog.length > 3) {
        var RangeDataLatest = sheet_target.getRange('F1');
        RangeDataLatest.setValue(rowDataLog[2]);
        return ContentService.createTextOutput('Success');
      } else {
        Logger.log('Error: rowDataLog is not valid');
        return ContentService.createTextOutput('Error: Invalid data');
      }
    }

    if (sts_val == 'writelog') {
      sheet_name = "DIEM_DANH";
      sheet_target = sheet_open.getSheetByName(sheet_name);
      Logger.log(JSON.stringify(rowDataLog));
      sheet_target.insertRows(2);
      var newRangeDataLog = sheet_target.getRange(2,1,1, rowDataLog.length);
      newRangeDataLog.setValues([rowDataLog]);
      return ContentService.createTextOutput(result);
    }

    if (sts_val == 'read') {
      sheet_name = "DANH_SACH";
      sheet_target = sheet_open.getSheetByName(sheet_name);
      var all_Data = sheet_target.getRange('A2:D11').getDisplayValues();
      return ContentService.createTextOutput(all_Data);
    }
  }
}

function maxRowData(allRowsAfter) {
  const sheet = SpreadsheetApp.getActiveSpreadsheet()
                              .getSheetByName('DATA');
  sheet.getRange(allRowsAfter+1, 1, sheet.getLastRow()-allRowsAfter, sheet.getLastColumn())
       .clearContent();
}

function stripQuotes(value) {
  return value.replace(/^\"|\"$/g, "");
}
