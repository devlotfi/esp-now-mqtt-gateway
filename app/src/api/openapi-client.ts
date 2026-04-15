import createFetchClient from "openapi-fetch";
import createClient from "openapi-react-query";
import type { paths } from "../__generated__/schema";
import type { cloudflarePaths } from "../__generated__/cloudflare-schema";

export const fetchClient = createFetchClient<paths>();
export const $api = createClient(fetchClient);

export const cloudflareFetchClient = createFetchClient<cloudflarePaths>();
export const $cloudflareApi = createClient(cloudflareFetchClient);
