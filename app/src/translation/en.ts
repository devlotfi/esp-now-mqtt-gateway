import type { AppTranslation } from "../types/app-translation";

export const EN: AppTranslation = {
  // Navigation
  dashboard: "Dashboard",
  settings: "Settings",
  information: "Information",

  // General actions
  save: "Save",
  cancel: "Cancel",
  add: "Add",
  edit: "Edit",
  delete: "Delete",
  allow: "Allow",
  test: "Test",
  notFound: "Page not found",
  landingPage: "Home",

  // User / Profile
  name: "Name",
  nameDescription: "The name that will be shown in the alert E-mail",
  nameSaved: "Name saved",
  email: "Email",
  systemDisabled: "System is disabled",
  alertsWillNotBesent: "Alerts will not be sent",
  nameMissing: "Name is missing",

  // API / Config
  apiKey: "API Key",
  apiKeyDescription: "Enter your API key",
  apiKeySaved: "API key saved",
  baseUrl: "Base URL",
  baseUrlDescription: "Enter the base URL",
  baseUrlSaved: "Base URL saved",
  apiUrl: "API URL",
  apiSecret: "API Secret",
  vapidPublicKey: "VAPID Public Key",

  // Lists / States
  emptyList: "No items found",

  // Alerts / Timer
  nextAlert: "Next alert",
  alertCountdown: "Alert countdown",
  timerElapsed: "Time elapsed",
  timerUpdated: "Timer updated",

  // Notifications
  notifications1: "Enable notifications",
  notifications2: "You will receive alerts and updates",

  // Errors / Status
  connectionError: "Connection error",
  permissionRequired: "Permission required",
  unknownError: "An unknown error occurred",
  requestError: "Request failed",
  error: "Error",
  actionSuccess: "Action completed successfully",

  // Appearance
  theme: "Theme",
  system: "System",
  light: "Light",
  dark: "Dark",
  accentColor: "Accent color",
  language: "Language",
  display: "Display",

  // API Subscription
  apiSubscription: "API Subscription",
  subscribe: "Subscribe",
  unsubscribe: "Unsubscribe",
  apiSubscribed: "Subscribed to API",
  apiNotSubscribed: "Not subscribed to API",
  subscribeApi1: "Subscribe to an API",
  subscribeApi2: "Ensure your API settings are correct",

  // Contacts
  contacts: "Contacts",
  enabled: "Enabled",
  enabledDescription: "Toggle to enable or disable the alert countdown",
  addContact: "Add contact",
  deleteContact: "Delete contact",
  deleteConfirmation: "Are you sure you want to delete this?",
  editContact: "Edit contact",
};
