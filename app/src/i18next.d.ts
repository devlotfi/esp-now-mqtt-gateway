import "i18next";
import { AppTranslation } from "./types/app-translation";

declare module "i18next" {
  interface CustomTypeOptions {
    resources: {
      translation: AppTranslation;
    };
  }
}
