import type { AppTranslation } from "../types/app-translation";

export const FR: AppTranslation = {
  // Navigation
  dashboard: "Tableau de bord",
  settings: "Paramètres",
  information: "Informations",

  // General actions
  save: "Enregistrer",
  cancel: "Annuler",
  add: "Ajouter",
  edit: "Modifier",
  delete: "Supprimer",
  allow: "Autoriser",
  test: "Tester",
  notFound: "Page introuvable",
  landingPage: "Accueil",

  // User / Profile
  name: "Nom",
  enabledDescription: "Activer ou désactiver le compte à rebours des alertes",
  nameSaved: "Nom enregistré",
  email: "Email",
  systemDisabled: "Le système est désactivé",
  alertsWillNotBesent: "Les alertes ne seront pas envoyées",
  nameMissing: "Nom manquant",

  // API / Config
  apiKey: "Clé API",
  apiKeyDescription: "Entrez votre clé API",
  apiKeySaved: "Clé API enregistrée",
  baseUrl: "URL de base",
  baseUrlDescription: "Entrez l’URL de base",
  baseUrlSaved: "URL de base enregistrée",
  apiUrl: "URL API",
  apiSecret: "Secret API",
  vapidPublicKey: "Clé publique VAPID",

  // Lists / States
  emptyList: "Aucun élément",

  // Alerts / Timer
  nextAlert: "Prochaine alerte",
  alertCountdown: "Compte à rebours",
  timerElapsed: "Temps écoulé",
  timerUpdated: "Minuteur mis à jour",

  // Notifications
  notifications1: "Activer les notifications",
  notifications2: "Vous recevrez des alertes et des mises à jour",

  // Errors / Status
  connectionError: "Erreur de connexion",
  permissionRequired: "Permission requise",
  unknownError: "Une erreur inconnue s’est produite",
  requestError: "Échec de la requête",
  error: "Erreur",
  actionSuccess: "Action réussie",

  // Appearance
  theme: "Thème",
  system: "Système",
  light: "Clair",
  dark: "Sombre",
  accentColor: "Couleur d’accent",
  language: "Langue",
  display: "Affichage",

  // API Subscription
  apiSubscription: "Abonnement API",
  subscribe: "S’abonner",
  unsubscribe: "Se désabonner",
  apiSubscribed: "Abonné à l’API",
  apiNotSubscribed: "Non abonné à l’API",
  subscribeApi1: "S’abonner à une API",
  subscribeApi2: "Vérifiez vos paramètres API",

  // Contacts
  contacts: "Contacts",
  enabled: "Activé",
  nameDescription: "Nom affiché dans l’e-mail d’alerte",
  addContact: "Ajouter un contact",
  deleteContact: "Supprimer le contact",
  deleteConfirmation: "Voulez-vous vraiment supprimer ceci ?",
  editContact: "Modifier le contact",
};
