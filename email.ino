// email.ino: sends emails

#include <ESP_Mail_Client.h>

#define SMTP_HOST "smtp.hostinger.com"
#define SMTP_PORT 465

#define SENDER_NAME     "Basement Monitor"
#define SENDER_EMAIL    "XXX"
#define SENDER_PASSWORD "XXX"
#define TARGET_NAME     "XXX"
#define TARGET_SMS      "XXX@msg.fi.google.com"
#define TARGET_EMAIL    "XXX"

SMTPSession smtp;
void smtpCallback(SMTP_Status status);

int lastReportedSeverity = 0;                 // water level severity from WaterSensor.ino
unsigned long lastAlertTime = 0;              // time when last alert was sent, in millis()
unsigned long maxMillis = 40*24*3600*1000;    // millis in 40 days
unsigned long resetTime = 6*3600*1000;        // 6 hours.  Reset alert flag

void setupEmail() {
  // empty
}

// call on each loop
void loopEmail() {
  // restart ESP 40 days after it booted.  This keeps the millis clock within bounds
  if (severity == 0  &&  millis() > maxMillis)
    ESP.restart();

  if (lastReportedSeverity > 0)     // resend alerts after 6 hours if preblem persists
    if ((millis() - lastAlertTime) > resetTime)
      lastReportedSeverity = 0;

  if (severity > lastReportedSeverity) {
    sendEmail(TARGET_NAME, TARGET_SMS);
    sendEmail(TARGET_NAME, TARGET_EMAIL);
  }
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status) {
  Serial.println(status.info());      /* Print the current status */

  /* Print the sending result */
  if (status.success()) {
    // update times to avoid excessive alerts
    lastReportedSeverity = severity;
    lastAlertTime = millis();

    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());

    for (size_t i = 0; i < smtp.sendingResult.size(); i++) {
      struct tm dt;
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t) result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients);
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject);
    }
  }
}

void sendEmail(String recipientName, String recipientEmail) {
 /** Enable the debug via Serial port
   * none debug or 0
   * basic debug or 1
  */
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the session config data */
  ESP_Mail_Session session;

  /* Set the session config */
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = SENDER_EMAIL;
  session.login.password = SENDER_PASSWORD;
  session.login.user_domain = "";

  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = SENDER_NAME;          // "Basement Monitor";
  message.sender.email = SENDER_EMAIL;        // From
  message.subject = "Basement Monitor Alert";
  message.addRecipient(recipientName, recipientEmail);

  //Send raw text message
  message.text.content = waterMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  /* Connect to server with the session config */
  if (!smtp.connect(&session))
    return;

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());
}