import { Component, OnInit } from '@angular/core';
import { AngularFireDatabase, AngularFireObject, AngularFireList } from '@angular/fire/compat/database';
import { Observable } from 'rxjs';
import { ESP32_DATA } from 'src/models/models';

@Component({
  selector: 'app-root',
  templateUrl: './index.html',
  styleUrls: ['./style.css']
})


export class AppComponent implements OnInit {
  title = 'Connected House';
  private fichierRef: AngularFireObject<ESP32_DATA>;
  public fichier?: Observable<any>;
  showAccessStatus = true;
  
  constructor(db: AngularFireDatabase) {
    this.fichierRef = db.object('ESP32_DATA');
    this.fichier = this.fichierRef.valueChanges();
   
  }
  private prevCardID: string | undefined;

  // checkCardIDChange(data: any): void {
  //   if (data.cardID !== undefined && data.cardID !== this.prevCardID) {
  //     this.showAccessStatus = true;
  //     this.prevCardID = data.cardID;
  //   }
  // }

  ngOnInit(): void {
    this.fichierRef.snapshotChanges().subscribe((data) => {
      if (data.payload.exists()) {
        const value = data.payload.val();
        console.log(value);
        // this.checkCardIDChange(value);

      

        // setTimeout(() => {
        //   this.showAccessStatus = false;
        // }, 200000);
      } else {
        console.log("Node does not exist or there is no data in the node.");
      }
    });
  }
}


