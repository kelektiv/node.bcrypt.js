declare module "bcrypt" {
    export function genSaltSync(rounds?: number, seed_length?: number):string;
    export function genSalt(rounds: number, seed_length: number, callback: (err: any, salt: string) => void);
    export function genSalt(callback: (err: any, salt: string) => void);
    export function hashSync(data: string, salt: string): string;
    export function hash(data:string, salt:string, callback: (err: any, hash: string) => void);
    export function compareSync(data:string, encrypted:string):boolean;
    export function compare(data: string, encrypted: string, callback: (err: any, matched: boolean) => void);
    export function getRounds(encrypted:string):number;
}
