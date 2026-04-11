import type { AppTranslation } from "../types/app-translation";

export const AR: AppTranslation = {
  // Navigation
  dashboard: "لوحة التحكم",
  settings: "الإعدادات",
  information: "معلومات",

  // General actions
  save: "حفظ",
  cancel: "إلغاء",
  add: "إضافة",
  edit: "تعديل",
  delete: "حذف",
  allow: "سماح",
  test: "اختبار",
  notFound: "الصفحة غير موجودة",
  landingPage: "الصفحة الرئيسية",

  // User / Profile
  name: "الاسم",
  enabledDescription: "تفعيل أو تعطيل العد التنازلي للتنبيه",
  nameSaved: "تم حفظ الاسم",
  email: "البريد الإلكتروني",
  systemDisabled: "النظام معطل",
  alertsWillNotBesent: "لن يتم إرسال التنبيهات",
  nameMissing: "الاسم مفقود",

  // API / Config
  apiKey: "مفتاح API",
  apiKeyDescription: "أدخل مفتاح API",
  apiKeySaved: "تم حفظ مفتاح API",
  baseUrl: "الرابط الأساسي",
  baseUrlDescription: "أدخل الرابط الأساسي",
  baseUrlSaved: "تم حفظ الرابط الأساسي",
  apiUrl: "رابط API",
  apiSecret: "سر API",
  vapidPublicKey: "المفتاح العام VAPID",

  // Lists / States
  emptyList: "لا توجد عناصر",

  // Alerts / Timer
  nextAlert: "التنبيه التالي",
  alertCountdown: "العد التنازلي",
  timerElapsed: "الوقت المنقضي",
  timerUpdated: "تم تحديث المؤقت",

  // Notifications
  notifications1: "تفعيل الإشعارات",
  notifications2: "ستتلقى تنبيهات وتحديثات",

  // Errors / Status
  connectionError: "خطأ في الاتصال",
  permissionRequired: "الإذن مطلوب",
  unknownError: "حدث خطأ غير معروف",
  requestError: "فشل الطلب",
  error: "خطأ",
  actionSuccess: "تمت العملية بنجاح",

  // Appearance
  theme: "المظهر",
  system: "النظام",
  light: "فاتح",
  dark: "داكن",
  accentColor: "لون التمييز",
  language: "اللغة",
  display: "العرض",

  // API Subscription
  apiSubscription: "اشتراك API",
  subscribe: "اشتراك",
  unsubscribe: "إلغاء الاشتراك",
  apiSubscribed: "تم الاشتراك في API",
  apiNotSubscribed: "غير مشترك في API",
  subscribeApi1: "الاشتراك في API",
  subscribeApi2: "تأكد من إعدادات API",

  // Contacts
  contacts: "جهات الاتصال",
  enabled: "مفعل",
  nameDescription: "الاسم الذي سيظهر في رسالة التنبيه عبر البريد الإلكتروني",
  addContact: "إضافة جهة اتصال",
  deleteContact: "حذف جهة الاتصال",
  deleteConfirmation: "هل أنت متأكد من الحذف؟",
  editContact: "تعديل جهة الاتصال",
};
